
#include "SkinnedModel.h"
#include "Helpers/Util.h"
//#include "Helpers/AssetManager.h"
//#include "Logic/WeaponLogic.h"
#include "Renderer/Renderer.h"

SkinnedModel::SkinnedModel()
{
    m_VAO = 0;
    ZERO_MEM(m_Buffers);
    m_NumBones = 0;
}

SkinnedModel::~SkinnedModel()
{
}

void SkinnedModel::Render(Shader* shader, const glm::mat4& modelMatrix)
{
    glBindVertexArray(m_VAO);
    shader->setMat4("model", modelMatrix);

    for (int i = 0; i < m_meshEntries.size(); i++) {            
        glDrawElementsBaseVertex(GL_TRIANGLES, m_meshEntries[i].NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * m_meshEntries[i].BaseIndex), m_meshEntries[i].BaseVertex);
    }
}

void SkinnedModel::UpdateBoneTransform(std::vector<glm::mat4>& Transforms, std::vector<glm::mat4>& DebugAnimatedTransforms, Ragdoll* ragdoll)
{
    return;

    // Traverse the tree 
    for (int i = 0; i < m_skeleton.m_joints.size(); i++)
    {
        // Get the node and its um bind pose transform?
        const char* NodeName = m_skeleton.m_joints[i].m_name;
        glm::mat4 NodeTransformation = m_skeleton.m_joints[i].m_inverseBindTransform;

        unsigned int parentIndex = m_skeleton.m_joints[i].m_parentIndex;

        glm::mat4 ParentTransformation = (parentIndex == -1) ? glm::mat4(1) : m_skeleton.m_joints[parentIndex].m_currentFinalTransform;
        glm::mat4 GlobalTransformation = ParentTransformation * NodeTransformation;

        m_skeleton.m_joints[i].m_currentFinalTransform = GlobalTransformation;

        if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
            unsigned int BoneIndex = m_BoneMapping[NodeName];
            m_BoneInfo[BoneIndex].FinalTransformation = GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
            m_BoneInfo[BoneIndex].ModelSpace_AnimatedTransform = GlobalTransformation;

            if (ragdoll != NULL)
            {
                btTransform worldTransform;

                RigidComponent* rigidComponent = ragdoll->GetRigidByName(NodeName);

                if (rigidComponent != NULL) {
                    rigidComponent->rigidBody->getMotionState()->getWorldTransform(worldTransform);

                    glm::mat4 matrix = Util::btTransformToMat4(worldTransform);
                    m_BoneInfo[BoneIndex].FinalTransformation = matrix * m_BoneInfo[BoneIndex].BoneOffset;
                    m_skeleton.m_joints[i].m_currentFinalTransform = matrix;
                    m_BoneInfo[BoneIndex].ModelSpace_AnimatedTransform = matrix;
                }
            }
        }
    }

    for (unsigned int i = 0; i < m_NumBones; i++) {
        Transforms[i] = m_BoneInfo[i].FinalTransformation;
        DebugAnimatedTransforms[i] = m_BoneInfo[i].ModelSpace_AnimatedTransform;
    }
}

int SkinnedModel::FindAnimatedNodeIndex(float AnimationTime, const AnimatedNode* animatedNode)
{
    // bail if current animation time is earlier than the this nodes first keyframe time
    if (AnimationTime < animatedNode->m_nodeKeys[0].timeStamp)
        return -1;

    for (unsigned int i = 0; i < animatedNode->m_nodeKeys.size() - 1; i++) {
        if (AnimationTime < animatedNode->m_nodeKeys[i + 1].timeStamp)
            return i;
    }
    return -1;
}


void SkinnedModel::CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const AnimatedNode* animatedNode)
{
    int PositionIndex = FindAnimatedNodeIndex(AnimationTime, animatedNode);
    int NextPositionIndex = (PositionIndex + 1);

    // Nothing to report
    if (PositionIndex == -1 || animatedNode->m_nodeKeys.size() == 1) {
        Out = animatedNode->m_nodeKeys[0].positon;
        return;
    }       
    float DeltaTime = animatedNode->m_nodeKeys[NextPositionIndex].timeStamp - animatedNode->m_nodeKeys[PositionIndex].timeStamp;
    float Factor = (AnimationTime - animatedNode->m_nodeKeys[PositionIndex].timeStamp) / DeltaTime;

    glm::vec3 start = animatedNode->m_nodeKeys[PositionIndex].positon;
    glm::vec3 end = animatedNode->m_nodeKeys[NextPositionIndex].positon;
    glm::vec3 delta = end - start;
    Out = start + Factor * delta;
}


void SkinnedModel::CalcInterpolatedRotation(glm::quat& Out, float AnimationTime, const AnimatedNode* animatedNode)
{
    int Index = FindAnimatedNodeIndex(AnimationTime, animatedNode);
    int NextIndex = (Index + 1);

    // Nothing to report
    if (Index == -1 || animatedNode->m_nodeKeys.size() == 1) {
        Out = animatedNode->m_nodeKeys[0].rotation;
        return;
    }
    float DeltaTime = animatedNode->m_nodeKeys[NextIndex].timeStamp - animatedNode->m_nodeKeys[Index].timeStamp;
    float Factor = (AnimationTime - animatedNode->m_nodeKeys[Index].timeStamp) / DeltaTime;

    const glm::quat& StartRotationQ = animatedNode->m_nodeKeys[Index].rotation;
    const glm::quat& EndRotationQ = animatedNode->m_nodeKeys[NextIndex].rotation;

    Util::InterpolateQuaternion(Out, StartRotationQ, EndRotationQ, Factor);
    Out = glm::normalize(Out);
}


void SkinnedModel::CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const AnimatedNode* animatedNode)
{
    int Index = FindAnimatedNodeIndex(AnimationTime, animatedNode);
    int NextIndex = (Index + 1);

  //  std::cout << "scale index:     " << Index << "\n";
 //   std::cout << "scale NextIndex: " << NextIndex << "\n";

    // Nothing to report
    if (Index == -1 || animatedNode->m_nodeKeys.size() == 1) {
        Out = glm::vec3(animatedNode->m_nodeKeys[0].scale);
        return;
    }
    float DeltaTime = animatedNode->m_nodeKeys[NextIndex].timeStamp - animatedNode->m_nodeKeys[Index].timeStamp;
    float Factor = (AnimationTime - animatedNode->m_nodeKeys[Index].timeStamp) / DeltaTime;

    glm::vec3 start = glm::vec3(animatedNode->m_nodeKeys[Index].scale);
    glm::vec3 end = glm::vec3(animatedNode->m_nodeKeys[NextIndex].scale);
    glm::vec3 delta = end - start;
    Out = start + Factor * delta;

/*
    std::cout << "m_nodeKeys.size():     " << animatedNode->m_nodeKeys.size() << "\n";
    std::cout << "timestamp:     " << animatedNode->m_nodeKeys[Index].timeStamp << "\n";

    std::cout << "DeltaTime:     " << DeltaTime << "\n";
    std::cout << "Factor: " << Factor << "\n";
    std::cout << "start:   " << start.x << ", " << start.y << ", " << start.z << "\n";
    std::cout << "end:     " << end.x << ", " << end.y << ", " << end.z << "\n";
    std::cout << "delta:   " << delta.x << ", " << delta.y << ", " << delta.z << "\n";
    */
}


void SkinnedModel::BoneTransform(float AnimationTime, unsigned int animIndex, std::vector<glm::mat4>& Transforms, std::vector<glm::mat4>& DebugAnimatedTransforms)
{
  /*  float TimeInSeconds = 0.24f;

    // Get the animation time
    float AnimationTime = 0;
    if (m_animations.size() > 0) {
        Animation* animation = m_animations[currentAnimationIndex];

        float TicksPerSecond = animation->m_ticksPerSecond != 0 ? animation->m_ticksPerSecond : 25.0f;
        float TimeInTicks = TimeInSeconds * TicksPerSecond;
        AnimationTime = fmod(TimeInTicks, animation->m_duration);
    }
   // std::cout << "\n" << m_filename << "\n";
    
    */
    std::vector<glm::mat4> tempParentFinalTransforms;
    for (int i = 0; i < m_skeleton.m_joints.size(); i++)
        tempParentFinalTransforms.push_back(glm::mat4(1));

    // Traverse the tree 
    for (int i = 0; i < m_skeleton.m_joints.size(); i++)
    {
        if (m_skeleton.m_joints.size() == 90)
        {
       //     std::cout << "\ni: " << i << "\n";
        }

        // Get the node and its um bind pose transform?
        const char* NodeName = m_skeleton.m_joints[i].m_name;
     //   std::cout << "NodeName " << NodeName << "\n";
        glm::mat4 NodeTransformation = m_skeleton.m_joints[i].m_inverseBindTransform;

        // Calculate any animation
        if (m_animations.size() > 0)
        {
             const AnimatedNode* animatedNode = FindAnimatedNode(m_animations[currentAnimationIndex], NodeName);

            if (animatedNode)
            {
                glm::vec3 Scaling;

                CalcInterpolatedScaling(Scaling, AnimationTime, animatedNode);
                glm::mat4 ScalingM;

                ScalingM = Util::Mat4InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);
                glm::quat RotationQ;
                CalcInterpolatedRotation(RotationQ, AnimationTime, animatedNode);
                glm::mat4 RotationM(RotationQ);

                glm::vec3 Translation;
                CalcInterpolatedPosition(Translation, AnimationTime, animatedNode);
                glm::mat4 TranslationM;

                TranslationM = Util::Mat4InitTranslationTransform(Translation.x, Translation.y, Translation.z);
                NodeTransformation = TranslationM * RotationM * ScalingM;
/*
                std::cout << "ScalingM\n";

                Util::PrintVec3(Scaling);// Util::PrintMat4(ScalingM);
                std::cout << "RotationQ\n";
                std::cout << "(" << RotationQ.x << ", " << RotationQ.y << ", " << RotationQ.z << ", " << RotationQ.w << "\n";
                Util::PrintVec3(Translation);
           
            std::cout << "TranslationM\n";

            Util::PrintVec3(Scaling);// Util::PrintMat4(ScalingM);*/
            }
        }
        int parentIndex = m_skeleton.m_joints[i].m_parentIndex;
  //    std::cout << "parentIndex " << parentIndex << "\n";
//
        glm::mat4 ParentTransformation = (parentIndex == -1) ? glm::mat4(1) : tempParentFinalTransforms[parentIndex];
        /*
        std::cout << "Node Transform\n";
        Util::PrintMat4(NodeTransformation);
        std::cout << "Parent Transform\n";
        Util::PrintMat4(ParentTransformation);*/
        
        glm::mat4 GlobalTransformation = ParentTransformation * NodeTransformation;

        tempParentFinalTransforms[i] = GlobalTransformation;

       /* if (parentIndex == 4) {
            std::cout << "i " << i << "\n";
            std::cout << "parentIndex " << parentIndex << "\n";
            Util::PrintMat4(NodeTransformation);
        }
        */
        if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
            unsigned int BoneIndex = m_BoneMapping[NodeName];
            m_BoneInfo[BoneIndex].FinalTransformation = GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
            m_BoneInfo[BoneIndex].ModelSpace_AnimatedTransform = GlobalTransformation;

 
            // If there is no bind pose, then just use bind pose
            // ???? How about you check if this does anything useful ever ????
            if (m_animations.size() == 0) {
                m_BoneInfo[BoneIndex].FinalTransformation = GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
                m_BoneInfo[BoneIndex].ModelSpace_AnimatedTransform = GlobalTransformation;
            }
        }
    }

    for (unsigned int i = 0; i < m_NumBones; i++) {
        Transforms[i] = m_BoneInfo[i].FinalTransformation;
        DebugAnimatedTransforms[i] = m_BoneInfo[i].ModelSpace_AnimatedTransform;
    }
    /*animIndex = 0;
    animationTime = 1;

    // Traverse the tree 
    for (int i = 0; i < m_skeleton.m_joints.size(); i++)
    {
        // Get the node and its um bind pose transform?
        const char* NodeName = m_skeleton.m_joints[i].m_name;
        glm::mat4 NodeTransformation = m_skeleton.m_joints[i].m_inverseBindTransform;

        // Calculate any animation

       // if (m_skeleton.m_joints.size() > 0 &&  m_skeleton.m_joints[i].m_parentIndex == 0)
        if (m_animations.size() > 0)// if (m_animations.size() > 0)
        {

            const AnimatedNode* animatedNode = FindAnimatedNode(m_animations[animIndex], NodeName);


            if (i == 1)
                std::cout << "node name: " << NodeName << "\n";

           


            if (animatedNode) {
                glm::vec3 Scaling;
                CalcInterpolatedScaling(Scaling, animationTime, animatedNode);
                glm::mat4 ScalingM;
                ScalingM = Util::Mat4InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);
                glm::quat RotationQ;
                CalcInterpolatedRotation(RotationQ, animationTime, animatedNode);
                glm::mat4 RotationM(RotationQ);
                glm::vec3 Translation;
                CalcInterpolatedPosition(Translation, animationTime, animatedNode);
                glm::mat4 TranslationM;
                TranslationM = Util::Mat4InitTranslationTransform(Translation.x, Translation.y, Translation.z);
                NodeTransformation = TranslationM * RotationM * ScalingM;


                if (i == 1)
                    Util::PrintMat4(TranslationM);

            }
            else
                if (i == 1)
                    std::cout << "node name: " << NodeName << " is unaanimated\n";


        }
        unsigned int parentIndex = m_skeleton.m_joints[i].m_parentIndex;
        glm::mat4 ParentTransformation = (parentIndex == -1) ? glm::mat4(1) : m_skeleton.m_joints[parentIndex].m_currentFinalTransform;
        glm::mat4 GlobalTransformation = ParentTransformation * NodeTransformation;

        // Store the current transformation, so child nodes can access it
        m_skeleton.m_joints[i].m_currentFinalTransform = GlobalTransformation;

        if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
            unsigned int BoneIndex = m_BoneMapping[NodeName];
            m_BoneInfo[BoneIndex].FinalTransformation = GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
            m_BoneInfo[BoneIndex].ModelSpace_AnimatedTransform = GlobalTransformation;

            // If there is no bind pose, then just use bind pose
            // ???? How about you check if this does anything useful ever ????
            if (m_animations.size() == 0) {
                m_BoneInfo[BoneIndex].FinalTransformation = GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
                m_BoneInfo[BoneIndex].ModelSpace_AnimatedTransform = GlobalTransformation;
            }
        }
    }

    for (unsigned int i = 0; i < m_NumBones; i++) {
        Transforms[i] = m_BoneInfo[i].FinalTransformation;
        DebugAnimatedTransforms[i] = m_BoneInfo[i].ModelSpace_AnimatedTransform;
    } */
}




const AnimatedNode* SkinnedModel::FindAnimatedNode(Animation* animation, const char* NodeName)
{
   /* std::cout << "\n";
    std::cout << "\n";
    std::cout << "\n";


    std::cout << "there are " << animation->m_animatedNodes.size() << " animatedn nodes. They are: \n";
*/
    for (unsigned int i = 0; i < animation->m_animatedNodes.size(); i++) {
        const AnimatedNode* animatedNode = &animation->m_animatedNodes[i];
      //  std::cout << " " << animation->m_animatedNodes[i].m_nodeName << " with " << animation->m_animatedNodes[i].m_nodeKeys.size() << " keys \n"; 

        if (Util::StrCmp(animatedNode->m_nodeName, NodeName)) {
        //    std::cout << "YOU FOUND THIS MATCH INCORECTRLY PROBABLY: " << NodeName << "\n";
            return animatedNode;
        }
    }
    return nullptr;
}