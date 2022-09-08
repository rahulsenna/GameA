//
// Created by AgentOfChaos on 4/8/2022.
//


#include "animation.h"


/* Gets the current index on mKeyPositions to interpolate to based on
    the current animation time*/
static int
GetPositionIndex(Bone *bone, r32 animationTime)
{
    for (int index = 0; index < bone->m_NumPositions - 1; ++index)
    {
        if (animationTime < bone->m_Positions[index + 1].timeStamp)
            return index;
    }
    assert(0);
    return 0;
}

/* Gets the current index on mKeyRotations to interpolate to based on the
    current animation time*/
static int
GetRotationIndex(Bone *bone, r32 animationTime)
{
    for (int index = 0; index < bone->m_NumRotations - 1; ++index)
    {
        if (animationTime < bone->m_Rotations[index + 1].timeStamp)
            return index;
    }
    assert(0);
    return 0;
}

/* Gets the current index on mKeyScalings to interpolate to based on the
    current animation time */
static int
GetScaleIndex(Bone *bone, r32 animationTime)
{
    for (int index = 0; index < bone->m_NumScalings - 1; ++index)
    {
        if (animationTime < bone->m_Scales[index + 1].timeStamp)
            return index;
    }
    assert(0);
    return 0;
}

/* Gets normalized value for Lerp & Slerp*/
static r32
GetScaleFactor(r32 lastTimeStamp, r32 nextTimeStamp, r32 animationTime)
{
    r32 scaleFactor  = 0.0f;
    r32 midWayLength = animationTime - lastTimeStamp;
    r32 framesDiff   = nextTimeStamp - lastTimeStamp;
    scaleFactor        = midWayLength / framesDiff;
    return scaleFactor;
}

/*figures out which position keys to interpolate b/w and performs the interpolation
    and returns the translation matrix*/
static glm::mat4
InterpolatePosition(Bone *bone, r32 animationTime)
{
    if (1 == bone->m_NumPositions)
        return glm::translate(glm::mat4(1.0f), bone->m_Positions[0].position);

    int       p0Index       = GetPositionIndex(bone, animationTime);
    int       p1Index       = p0Index + 1;
    r32     scaleFactor   = GetScaleFactor(bone->m_Positions[p0Index].timeStamp,
                                             bone->m_Positions[p1Index].timeStamp, animationTime);
    glm::vec3 finalPosition = glm::mix(bone->m_Positions[p0Index].position,
                                       bone->m_Positions[p1Index].position, scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

/*figures out which rotations keys to interpolate b/w and performs the interpolation
    and returns the rotation matrix*/
static glm::mat4
InterpolateRotation(Bone *bone, r32 animationTime)
{
    if (1 == bone->m_NumRotations)
    {
        auto rotation = glm::normalize(bone->m_Rotations[0].orientation);
        return glm::toMat4(rotation);
    }

    int       p0Index       = GetRotationIndex(bone, animationTime);
    int       p1Index       = p0Index + 1;
    r32     scaleFactor   = GetScaleFactor(bone->m_Rotations[p0Index].timeStamp,
                                             bone->m_Rotations[p1Index].timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(bone->m_Rotations[p0Index].orientation,
                                         bone->m_Rotations[p1Index].orientation, scaleFactor);
    finalRotation           = glm::normalize(finalRotation);
    return glm::toMat4(finalRotation);
}

/*figures out which scaling keys to interpolate b/w and performs the interpolation
    and returns the scale matrix*/
static glm::mat4
InterpolateScaling(Bone *bone, r32 animationTime)
{
    if (1 == bone->m_NumScalings)
        return glm::scale(glm::mat4(1.0f), bone->m_Scales[0].scale);

    int       p0Index     = GetScaleIndex(bone, animationTime);
    int       p1Index     = p0Index + 1;
    r32     scaleFactor = GetScaleFactor(bone->m_Scales[p0Index].timeStamp,
                                           bone->m_Scales[p1Index].timeStamp, animationTime);
    glm::vec3 finalScale  = glm::mix(bone->m_Scales[p0Index].scale,
                                     bone->m_Scales[p1Index].scale, scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
}

/*interpolates  b/w positions,rotations & scaling keys based on the curren time of
    the animation and prepares the local transformation matrix by combining all keys
    tranformations*/
static void
boneUpdate(Bone *bone, r32 animationTime)
{
    // glm::mat4 translation  = InterpolatePosition(bone, animationTime);
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), bone->m_Positions[0].position);

    glm::mat4 rotation     = InterpolateRotation(bone, animationTime);
    glm::mat4 scale        = InterpolateScaling(bone, animationTime);
    bone->m_LocalTransform = translation * rotation * scale;
}

static Bone *
FindBone(Animation *anim, const std::string &name)
{
    auto iter = std::find_if(anim->Bones.begin(), anim->Bones.end(),
                             [&](const Bone &Bone) {
                             return Bone.m_Name == name;
                             });
    if (iter == anim->Bones.end()) return nullptr;
    else
        return &(*iter);
}
/*reads keyframes from aiNodeAnim*/
static Bone
makeBone(const std::string &name, int ID, const aiNodeAnim *channel)

{
    Bone bone             = {};
    bone.m_Name           = name;
    bone.m_ID             = ID;
    bone.m_LocalTransform = glm::mat4(1.0f);

    bone.m_NumPositions = channel->mNumPositionKeys;

    for (int positionIndex = 0; positionIndex < bone.m_NumPositions; ++positionIndex)
    {
        aiVector3D  aiPosition = channel->mPositionKeys[positionIndex].mValue;
        r32       timeStamp  = channel->mPositionKeys[positionIndex].mTime;
        KeyPosition data;
        data.position  = glmV3fromAiV3(aiPosition);
        data.timeStamp = timeStamp;
        bone.m_Positions.push_back(data);
    }

    bone.m_NumRotations = channel->mNumRotationKeys;
    for (int rotationIndex = 0; rotationIndex < bone.m_NumRotations; ++rotationIndex)
    {
        aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
        r32        timeStamp     = channel->mRotationKeys[rotationIndex].mTime;
        KeyRotation  data;
        data.orientation = GetGLMQuat(aiOrientation);
        data.timeStamp   = timeStamp;
        bone.m_Rotations.push_back(data);
    }

    bone.m_NumScalings = channel->mNumScalingKeys;
    for (int keyIndex = 0; keyIndex < bone.m_NumScalings; ++keyIndex)
    {
        aiVector3D scale     = channel->mScalingKeys[keyIndex].mValue;
        r32      timeStamp = channel->mScalingKeys[keyIndex].mTime;
        KeyScale   data;
        data.scale     = glmV3fromAiV3(scale);
        data.timeStamp = timeStamp;
        bone.m_Scales.push_back(data);
    }
    return (bone);
}

static void
ReadMissingBones(Animation *anim, const aiAnimation *animation, model *model)
{
    int size = animation->mNumChannels;

    auto &boneInfoMap = model->m_BoneInfoMap;//getting m_BoneInfoMap from Model class
    int  &boneCount   = model->m_BoneCounter;//getting the m_BoneCounter from Model class

    //reading channels(bones engaged in an animation and their keyframes)
    for (int i = 0; i < size; i++)
    {
        auto        channel  = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            boneInfoMap[boneName].id = boneCount;
            boneCount++;
        }
        anim->Bones.push_back(makeBone(channel->mNodeName.data,
                                       boneInfoMap[channel->mNodeName.data].id, channel));
    }

    anim->BoneInfoMap = boneInfoMap;
}

static void
ReadHeirarchyData(AssimpNodeData &dest, const aiNode *src)
{
    assert(src);

    dest.name           = src->mName.data;
    dest.transformation = ConvertMatrixToGLMFormat(src->mTransformation);
    dest.childrenCount  = src->mNumChildren;

    for (int i = 0; i < src->mNumChildren; i++)
    {
        AssimpNodeData newData;
        ReadHeirarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}

static Animation
makeAnimation(const std::string &animationPath, model *model, AnimationState state)
{
    Animation      anim;
    const aiScene *scene = aiImportFile(animationPath.c_str(), aiProcess_Triangulate);
    assert(scene && scene->mRootNode);
    auto animation                   = scene->mAnimations[0];
    anim.Duration                    = animation->mDuration;
    anim.TicksPerSecond              = animation->mTicksPerSecond;
    aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
    globalTransformation             = globalTransformation.Inverse();
    ReadHeirarchyData(anim.RootNode, scene->mRootNode);
    ReadMissingBones(&anim, animation, model);

    // For blending animations
    anim.state       = state;
    anim.blendFactor = 0.0f;
    return (anim);
}

static Animator
makeAnimator(Animation *currentAnimation)
{
    Animator animator;
    animator.CurrentTime      = 0.0;
    animator.CurrentAnimation = currentAnimation;

    animator.FinalBoneMatrices.reserve(MAX_BONES);

    for (int i = 0; i < MAX_BONES; i++)
    {
        animator.FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    return (animator);
}

static void
CalculateBoneTransform(Animator *animator, const AssimpNodeData *node, glm::mat4 parentTransform)
{
    std::string nodeName      = node->name;
    glm::mat4   nodeTransform = node->transformation;

    Bone *Bone = FindBone(animator->CurrentAnimation, nodeName);

    if (Bone)
    {
        boneUpdate(Bone, animator->CurrentTime);
        nodeTransform = Bone->m_LocalTransform;
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = animator->CurrentAnimation->BoneInfoMap;
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int       index                    = boneInfoMap[nodeName].id;
        glm::mat4 offset                   = boneInfoMap[nodeName].offset;
        animator->FinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < node->childrenCount; i++)
        CalculateBoneTransform(animator, &node->children[i], globalTransformation);
}

static void
UpdateAnimation(Animator *animator, r32 dt)
{
    animator->dt = dt;
    if (animator->CurrentAnimation)
    {
        animator->CurrentTime += animator->CurrentAnimation->TicksPerSecond * dt;
        animator->CurrentTime = fmod(animator->CurrentTime, animator->CurrentAnimation->Duration);
        CalculateBoneTransform(animator, &animator->CurrentAnimation->RootNode, glm::mat4(1.0f));
    }
}

static void
PlayAnimation(Animator *animator, Animation *pAnimation)
{
    animator->CurrentAnimation = pAnimation;
    animator->CurrentTime      = 0.0f;
}

// Recursive function that sets interpolated bone matrices in the 'm_FinalBoneMatrices' vector
static void
CalculateBlendedBoneTransform(Animator  *animator,
                              Animation *pAnimationBase, const AssimpNodeData *node,
                              Animation *pAnimationLayer, const AssimpNodeData *nodeLayered,
                              const r32 currentTimeBase, const r32 currentTimeLayered,
                              const glm::mat4 &parentTransform,
                              const r32      blendFactor)
{
    const std::string &nodeName = node->name;

    glm::mat4 nodeTransform = node->transformation;
    Bone     *pBone         = FindBone(pAnimationBase, nodeName);
    if (pBone)
    {
        boneUpdate(pBone, currentTimeBase);
        nodeTransform = pBone->m_LocalTransform;
    }

    glm::mat4 layeredNodeTransform = nodeLayered->transformation;
    pBone                          = FindBone(pAnimationLayer, nodeName);
    if (pBone)
    {
        boneUpdate(pBone, currentTimeLayered);
        layeredNodeTransform = pBone->m_LocalTransform;
    }

    // Blend two matrices
    const glm::quat rot0       = glm::quat_cast(nodeTransform);
    const glm::quat rot1       = glm::quat_cast(layeredNodeTransform);
    const glm::quat finalRot   = glm::slerp(rot0, rot1, blendFactor);
    glm::mat4       blendedMat = glm::mat4_cast(finalRot);
    blendedMat[3] =
    (1.0f - blendFactor) * nodeTransform[3] +
    layeredNodeTransform[3] * blendFactor;

    glm::mat4 globalTransformation = parentTransform * blendedMat;

    const auto &boneInfoMap = pAnimationBase->BoneInfoMap;
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        const int        index  = boneInfoMap.at(nodeName).id;
        const glm::mat4 &offset = boneInfoMap.at(nodeName).offset;

        animator->FinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (size_t i = 0; i < node->children.size(); ++i)
        CalculateBlendedBoneTransform(animator,
                                      pAnimationBase, &node->children[i],
                                      pAnimationLayer, &nodeLayered->children[i],
                                      currentTimeBase, currentTimeLayered, globalTransformation, blendFactor);
}

static void
BlendTwoAnimations(Animator  *animator,
                   Animation *BaseAnimation,
                   Animation *LayeredAnimation,
                   r32      blendFactor,
                   r32      dt)
{
    // Speed multipliers to correctly transition from one animation to another

    const r32 animSpeedMultiplierUp   = Lerp(1.f, blendFactor, BaseAnimation->Duration / LayeredAnimation->Duration);
    const r32 animSpeedMultiplierDown = Lerp(LayeredAnimation->Duration / BaseAnimation->Duration, blendFactor, 1.0f);

    // Current time of each animation, "scaled" by the above speed multiplier variables
    static r32 currentTimeBase = 0.0f;
    currentTimeBase += BaseAnimation->TicksPerSecond * dt * animSpeedMultiplierUp;
    currentTimeBase = fmod(currentTimeBase, BaseAnimation->Duration);

    static r32 currentTimeLayered = 0.0f;
    currentTimeLayered += LayeredAnimation->TicksPerSecond * dt * animSpeedMultiplierDown;
    currentTimeLayered = fmod(currentTimeLayered, LayeredAnimation->Duration);

    CalculateBlendedBoneTransform(animator,
                                  BaseAnimation, &BaseAnimation->RootNode,
                                  LayeredAnimation, &LayeredAnimation->RootNode,
                                  currentTimeBase, currentTimeLayered, glm::mat4(1.0f), blendFactor);
}

void CrossFadeAnimation(player *Player, Animator *animator, Animation *anim, r32 dt)
{
    r32 blendFactor = 1.f;
    if (Player->currentAnim->state != anim->state)
    {
        Player->prevAnim              = Player->currentAnim;
        Player->prevAnim->blendFactor = 0.f;
        Player->currentAnim           = anim;
    }
    r32 tFactor = 0.01f;

    if (Player->AnimState == Run)
    {
        tFactor = 0.05f;
    }
    if (Player->AnimState == Idle)
    {
        tFactor = 0.008f;
    }
    Player->currentAnim->blendFactor = glm::clamp(Player->currentAnim->blendFactor + tFactor, 0.f, 1.f);
    blendFactor                      = Player->currentAnim->blendFactor;

    BlendTwoAnimations(animator,
                       Player->prevAnim, anim,
                       blendFactor, dt);
}

AnimManager MakeAnimationManager(model *model, const char *Filename, AnimationState state)
{
    AnimManager manager = {};
    manager.animation   = makeAnimation(Filename, model, state);
    manager.animator    = makeAnimator(&manager.animation);

    return manager;
}