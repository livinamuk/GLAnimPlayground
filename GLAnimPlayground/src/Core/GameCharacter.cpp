#include "GameCharacter.h"
#include "Renderer/Texture.h"

GameCharacter::GameCharacter()
{
}

GameCharacter::~GameCharacter()
{
}

void GameCharacter::UpdateAnimation(float deltaTime)
{
    // SKIN THE MODEL
    m_animatedTransforms.resize(m_skinnedModel->m_BoneInfo.size());
    m_animatedDebugTransforms_Animated.resize(m_skinnedModel->m_BoneInfo.size());

    for (int i = 0; i < m_animatedTransforms.size(); i++)
    {
        m_animatedTransforms[i] = glm::mat4(1);
        m_animatedDebugTransforms_Animated[i] = glm::mat4(1);
    }

   // if (m_currentAnimationIndex == -1 || m_currentAnimationIndex >= m_skinnedModel->m_animations.size())
    //    return;

    //if (!m_pause)
    m_animationState.currentTime += deltaTime * m_animationState.speed;


  //  float currentAnimationDuration = m_skinnedModel->m_animations[m_currentAnimationIndex]->m_duration / 30.0f;
    float currentAnimationDuration = m_skinnedModel->m_animations[m_animationState.currentIndex]->m_duration / 30.0f;
    //std::cout << "DUR: " << currentAnimationDuration << "\n";
    // Looping
   
    //m_animationIsComplete = false;

    if (m_animationState.loop)
        if (m_animationState.currentTime > currentAnimationDuration)
            m_animationState.currentTime = 0;

    // No 
   /* if (!m_loopCurrentAnimation)
        if (m_currentAnimationTime >= m_currentAnimationDuration)
        {
            // Get timestamp of final frame
            // this might be dumb. think about it when youaren't tired.
            // its probably an error with the animation interpotion not here.
            m_currentAnimationTime = m_currentAnimationDuration - 0.01f;// skinnedModel->m_animations[m_currentAnimationIndex]->m_finalTimeStamp / skinnedModel->m_animations[m_currentAnimationIndex]->m_ticksPerSecond;
            m_animationIsComplete = true;
        }*/



        // Get the animation time
        float animationTime = 0;

        if (m_skinnedModel->m_animations.size() > 0) 
        {
            Animation* animation = m_skinnedModel->m_animations[m_animationState.currentIndex];

            if (animation) {
                float TicksPerSecond = animation->m_ticksPerSecond != 0 ? animation->m_ticksPerSecond : 25.0f;
                float TimeInTicks = m_animationState.currentTime * TicksPerSecond;
                animationTime = fmod(TimeInTicks, animation->m_duration);

               // std::cout <<"\nanimation->m_duration " << animation->m_duration << "\n";
          //      std::cout << "TimeInTicks " << TimeInTicks << "\n";
           //     std::cout << "AnimationTime " << animationTime << "\n";
            }
        }


    m_skinnedModel->BoneTransform(animationTime, m_animationState.currentIndex, m_animatedTransforms, m_animatedDebugTransforms_Animated);
   
    // Now set ragdoll to skin
}

void GameCharacter::RenderSkinnedModel(Shader* shader)
{   
    // SKIN THE MODEL
    m_animatedTransforms.resize(m_skinnedModel->m_BoneInfo.size());
    m_animatedDebugTransforms_Animated.resize(m_skinnedModel->m_BoneInfo.size());
    
    glm::mat4 modelMatrix;

    if (m_skinningMethod == SkinningMethod::ANIMATED)
    {
        modelMatrix = m_transform.to_mat4();

     //   m_skinnedModel->BoneTransform(0.5f, m_animationState.currentIndex, m_animatedTransforms, m_animatedDebugTransforms_Animated);

     //   m_skinnedModel->UpdateBoneTransform(m_animatedTransforms, m_animatedDebugTransforms_Animated, NULL);
    }
    else if (m_skinningMethod == SkinningMethod::RAGDOLL) {
        modelMatrix = glm::mat4(1);
        m_skinnedModel->UpdateBoneTransform(m_animatedTransforms, m_animatedDebugTransforms_Animated, &m_ragdoll);
        //return;
    }

   // Util::PrintMat4(m_animatedTransforms[0]);

   // return;

    shader->setInt("hasAnimation", true);

    for (unsigned int i = 0; i < m_animatedTransforms.size(); i++)
        shader->setMat4("skinningMats[" + std::to_string(i) + "]", modelMatrix * m_animatedTransforms[i]);

    glBindVertexArray(m_skinnedModel->m_VAO);
    glActiveTexture(GL_TEXTURE0);

    for (int i = 0; i < m_skinnedModel->m_meshEntries.size(); i++) {
        if (m_skinnedModel->m_meshEntries[i].Name == "Body_LOD0")
            glBindTexture(GL_TEXTURE_2D, Texture::GetTexIDByTexName("Zombie_Face_ALB.png"));
        if (m_skinnedModel->m_meshEntries[i].Name == "Tshirt_LOD0")
            glBindTexture(GL_TEXTURE_2D, Texture::GetTexIDByTexName("Zombie_Shirt_ALB.png"));
        if (m_skinnedModel->m_meshEntries[i].Name == "Hair_LOD0")
            glBindTexture(GL_TEXTURE_2D, Texture::GetTexIDByTexName("Zombie_Hair_ALB.png"));
        if (m_skinnedModel->m_meshEntries[i].Name == "BTM_LOD0")
            glBindTexture(GL_TEXTURE_2D, Texture::GetTexIDByTexName("Zombie_Jeans_ALB.png"));

        if (m_skinnedModel->m_meshEntries[i].Name == "Ch16_Body1")
            glBindTexture(GL_TEXTURE_2D, Texture::GetTexIDByTexName("Nurse_ALB.png"));
        if (m_skinnedModel->m_meshEntries[i].Name == "Ch16_Cap")
            glBindTexture(GL_TEXTURE_2D, Texture::GetTexIDByTexName("Nurse2_ALB.png"));
        if (m_skinnedModel->m_meshEntries[i].Name == "Ch16_Eyelashes")
            glBindTexture(GL_TEXTURE_2D, Texture::GetTexIDByTexName("Nurse_ALB.png"));
        if (m_skinnedModel->m_meshEntries[i].Name == "Ch16_Mask")
            glBindTexture(GL_TEXTURE_2D, Texture::GetTexIDByTexName("Nurse2_ALB.png"));
        if (m_skinnedModel->m_meshEntries[i].Name == "Ch16_Pants")
            glBindTexture(GL_TEXTURE_2D, Texture::GetTexIDByTexName("Nurse_ALB.png"));
        if (m_skinnedModel->m_meshEntries[i].Name == "Ch16_Shirt")
            glBindTexture(GL_TEXTURE_2D, Texture::GetTexIDByTexName("Nurse_ALB.png"));
        if (m_skinnedModel->m_meshEntries[i].Name == "Ch16_Shoes")
            glBindTexture(GL_TEXTURE_2D, Texture::GetTexIDByTexName("Nurse2_ALB.png"));

        glDrawElementsBaseVertex(GL_TRIANGLES, m_skinnedModel->m_meshEntries[i].NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * m_skinnedModel->m_meshEntries[i].BaseIndex), m_skinnedModel->m_meshEntries[i].BaseVertex);
    }

    shader->setInt("hasAnimation", false);
}
