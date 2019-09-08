#include "modconv.hxx"

#include "animconv.hxx"

#define YAW_AXIS y
#define PITCH_AXIS x
#define ROLL_AXIS z

#define PADDED_HEX(n, k) \
    "0x" \
    << std::setfill('0') \
    << std::setw((k)) \
    << std::hex \
    << (n)

std::string get_anim_name(aiAnimation *anim) {
    aiString rawName = anim->mName;
    std::regex rgx ("[^A-Za-z0-9_]");

    return "anim_" + std::regex_replace(rawName.C_Str(), rgx, "");
}

template<typename T> INLINE std::string padded_hex(T num) {
    return std::to_string(num);
}

template<typename TReal> INLINE s16 sm64(TReal angle) {
    return (s16)(angle * 32768.0 / M_PI);
}

template<typename TReal> void quat_to_sm64(aiQuaterniont<TReal> quat, aiVector3t<TReal> &vec) {
    TReal siny_cosp =  2.0 * (quat.w * quat.z + quat.y * quat.x);
    TReal cosy_cosp = -2.0 * (quat.z * quat.z + quat.y * quat.y) + 1.0;
    vec.YAW_AXIS = sm64(atan2(siny_cosp, cosy_cosp));

    TReal sinp = 2.0 * (quat.w * quat.y - quat.z * quat.x);
    vec.PITCH_AXIS = sm64(fabs(sinp) >= 1 ? copysign(M_PI / 2, sinp) : asin(sinp));

    TReal sinr_cosp =  2.0 * (quat.w * quat.x + quat.y * quat.z);
    TReal cosr_cosp = -2.0 * (quat.x * quat.x + quat.y * quat.y) + 1.0;
    vec.ROLL_AXIS = sm64(atan2(sinr_cosp, cosr_cosp));
}

template<typename TKey> void push_back_key_prop(std::vector<s16> &vec, TKey key, int prop) {
    vec.push_back((s16) key.mValue[prop]);
}

template<> void push_back_key_prop<aiQuatKey>(std::vector<s16> &vec, aiQuatKey key, int prop) {
    aiVector3t<TReal> rot;

    quat_to_sm64(key.mValue, rot);

    vec.push_back((s16) rot[prop]);
}

template<typename TKey> void add_keyframes_to_value_idx_tables(TKey keys[], unsigned int numKeys, std::vector<s16> &animValues, std::vector<s16> &animIndex) {
    for (int key = 0; key < 3; key++) {
        std::vector<s16> values;

        for (unsigned int i = 0; i < numKeys; i++) {
            push_back_key_prop(values, keys[i], key);
        }

        // If all values are the same, push 0x0001 entry
        if (std::equal(values.begin() + 1, values.end(), values.begin())) {
            animIndex.push_back(1);

            std::vector<s16>::iterator it = std::find(animValues.begin(), animValues.end(), values[0]);

            if (it == animValues.end()) {
                animIndex.push_back(animValues.size());
                animValues.push_back(values[0]);
            } else {
                animIndex.push_back(std::distance(animValues.begin(), it));
            }
        } else {
            animIndex.push_back(numKeys);
            animIndex.push_back(animValues.size());

            animValues.insert(animValues.end(), values.begin(), values.end());
        }
    }
}

void process_anim(std::string animName, const std::string fileOut, aiAnimation *anim, aiNode *rootNode) {
    std::string rawAnimName(anim->mName.C_Str());

    info_message("Processing animation " + rawAnimName);

    std::string filename = fileOut + "/" + animName + ".s";

    reset_file(filename);

    std::fstream animOut;
    animOut.open(filename, std::ofstream::out | std::ofstream::app);

    if (anim->mTicksPerSecond != 30) {
        warn_message("Animation " + rawAnimName + " is not 30 FPS (" + std::to_string(anim->mTicksPerSecond) + " FPS); this will result in slowed down or sped up animations.");
    }

    std::vector<s16> animValues;
    std::vector<s16> animIndex;

    for (unsigned int i = 0; i < anim->mNumChannels; i++) {
        const aiNodeAnim *nodeAnim = anim->mChannels[i];

        bool isAnimRoot = false;

        for (unsigned int j = 0; j < rootNode->mNumChildren; j++) {
            if (rootNode->mChildren[j]->mName == nodeAnim->mNodeName) {
                isAnimRoot = true;
            }
        }

        if (isAnimRoot) {
            add_keyframes_to_value_idx_tables(nodeAnim->mPositionKeys, nodeAnim->mNumPositionKeys, animValues, animIndex);
        }

        add_keyframes_to_value_idx_tables(nodeAnim->mRotationKeys, nodeAnim->mNumRotationKeys, animValues, animIndex);
    }

    animOut << animName << "_values:";

    for (long unsigned int i = 0; i < 12 * (animValues.size() / 12 + 1); i++) {
        if (i % 12 == 0) {
            animOut << std::endl << "    .hword ";
        } else {
            animOut << ", ";
        }

        animOut << PADDED_HEX(i >= animValues.size() ? (s16) -1 : animValues[i], 4);
    }

    animOut << std::endl << std::endl << animName << "_index:";

    for (long unsigned int i = 0; i < animIndex.size(); i++) {
        if (i == 0 || (i >= 12 && i % 6 == 0)) {
            animOut << std::endl << "    .hword ";
        } else {
            animOut << ", ";
        }

        animOut << PADDED_HEX(animIndex[i], 4);
    }

    animOut << std::endl << std::endl;

    animOut
        << animName << ":" << std::endl
        << "    .hword " << PADDED_HEX(0, 4) << " # flags" << std::endl
        << "    .hword " << PADDED_HEX(0, 4) << " # unk02" << std::endl
        << "    .hword " << PADDED_HEX(0, 4) << " # starting frame" << std::endl
        << "    .hword " << PADDED_HEX(0, 4) << " # loop starting frame" << std::endl
        << "    .hword " << PADDED_HEX(anim->mDuration, 4) << " # loop length" << std::endl
        << "    .hword " << PADDED_HEX(0, 4) << " # unused0A" << std::endl
        << "    .word " << animName << "_values" << std::endl
        << "    .word " << animName << "_index" << std::endl
        << "    .hword " << PADDED_HEX(0, 4) << " # length (only used in mario anims)" << std::endl;
}

void animconv_main(const std::string &file, const std::string &fileOut, bool level) {
    Assimp::Importer importer;

    const aiScene *scene = importer.ReadFile(file, aiProcess_ValidateDataStructure);

    reset_directory(fileOut);

    if (!scene->HasAnimations() || scene->mNumAnimations == 0) {
        error_message("Model has no animations.");
    }

    std::vector<std::string> animNames;

    for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
        aiAnimation *anim = scene->mAnimations[i];
        std::string animName = get_anim_name(anim);

        animNames.push_back(animName);

        process_anim(animName, fileOut, anim, scene->mRootNode);
    }

    std::string headerFilename = fileOut + "/anims.s";

    reset_file(headerFilename);

    std::fstream animHeaderOut;
    animHeaderOut.open(headerFilename, std::ofstream::out | std::ofstream::app);

    for (long unsigned int i = 0; i < animNames.size(); i++) {
        animHeaderOut << ".include \"" << (level ? "levels" : "actors") << "/" << fileOut << "/" << animNames[i] << ".s\"" << std::endl << std::endl;
    }

    animHeaderOut << "glabel " << fileOut << "_anims" << std::endl;

    for (long unsigned int i = 0; i < animNames.size(); i++) {
        animHeaderOut << "    .word " << animNames[i] << std::endl;
    }
}

#undef YAW_AXIS
#undef PITCH_AXIS
#undef ROLL_AXIS
