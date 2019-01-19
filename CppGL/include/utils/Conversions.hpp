#pragma once

#include <foundation/PxMat44.h>
#include <PxPhysics.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

inline physx::PxVec3 vec3ToPx(const glm::vec3& in)
{
    return physx::PxVec3(in.x, in.y, in.z);
}
inline glm::vec3 vec3ToGlm(const physx::PxVec3& in)
{
    return glm::vec3(in.x, in.y, in.z);
}
inline physx::PxQuat quatToPx(const glm::quat& in)
{
    return physx::PxQuat(in.x, in.y, in.z, in.w);
}
inline glm::quat quatToGlm(const physx::PxQuat& in)
{
    return glm::quat(in.w, in.x, in.y, in.z);
}
inline physx::PxMat44 mat44ToPx(const glm::mat4& in) {
    physx::PxMat44 mat;
    mat[0][0] = in[0][0];
    mat[0][1] = in[0][1];
    mat[0][2] = in[0][2];
    mat[0][3] = in[0][3];
    mat[1][0] = in[1][0];
    mat[1][1] = in[1][1];
    mat[1][2] = in[1][2];
    mat[1][3] = in[1][3];
    mat[2][0] = in[2][0];
    mat[2][1] = in[2][1];
    mat[2][2] = in[2][2];
    mat[2][3] = in[2][3];
    mat[3][0] = in[3][0];
    mat[3][1] = in[3][1];
    mat[3][2] = in[3][2];
    mat[3][3] = in[3][3];
    return mat;
}
inline glm::mat4 mat44ToGlm(const physx::PxMat44& in) {
    glm::mat4 mat;
    mat[0][0] = in[0][0];
    mat[0][1] = in[0][1];
    mat[0][2] = in[0][2];
    mat[0][3] = in[0][3];
    mat[1][0] = in[1][0];
    mat[1][1] = in[1][1];
    mat[1][2] = in[1][2];
    mat[1][3] = in[1][3];
    mat[2][0] = in[2][0];
    mat[2][1] = in[2][1];
    mat[2][2] = in[2][2];
    mat[2][3] = in[2][3];
    mat[3][0] = in[3][0];
    mat[3][1] = in[3][1];
    mat[3][2] = in[3][2];
    mat[3][3] = in[3][3];
    return mat;
}
