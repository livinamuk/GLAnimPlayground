#pragma once
#include "header.h"
#include "Animation/SkinnedModel.h"
#include "Physics/Ragdoll.h"

enum class SkinningMethod {RAGDOLL, BINDPOSE, ANIMATED};

struct AnimationState {
    bool loop = true;
    unsigned int currentIndex = 0;
    float currentTime = 0;
    float speed = 1.0f;
};

class GameCharacter
{
public:
    GameCharacter();
    ~GameCharacter();

    void UpdateAnimation(float deltaTime);
    void RenderSkinnedModel(Shader* shader);

    Transform m_transform;
    Ragdoll m_ragdoll;
    SkinnedModel* m_skinnedModel = nullptr;
    SkinningMethod m_skinningMethod = SkinningMethod::BINDPOSE;

    std::vector<glm::mat4> m_animatedTransforms; 
    std::vector<glm::mat4> m_animatedDebugTransforms_Animated;

    AnimationState m_animationState;
};