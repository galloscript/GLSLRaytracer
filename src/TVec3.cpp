

#include "Common.h"
#include "TVec3.h"

TVec3::TVec3(float aValue)
    : x(aValue)
    , y(aValue)
    , z(aValue)
{

}

TVec3::TVec3(float aX, float aY, float aZ)
    : x(aX)
    , y(aY)
    , z(aZ)
{

}

const TVec3& TVec3::operator+=(const TVec3& aVec)
{
    x += aVec.x;
    y += aVec.y;
    z += aVec.z;
    return *this;
}

const TVec3& TVec3::operator+=(float aScalar)
{
    x += aScalar;
    y += aScalar;
    z += aScalar;
    return *this;
}

const TVec3& TVec3::operator-=(const TVec3& aVec)
{
    x -= aVec.x;
    y -= aVec.y;
    z -= aVec.z;
    return *this;
}

const TVec3& TVec3::operator-=(float aScalar)
{
    x -= aScalar;
    y -= aScalar;
    z -= aScalar;
    return *this;
}

const TVec3& TVec3::operator*=(const TVec3& aVec)
{
    x *= aVec.x;
    y *= aVec.y;
    z *= aVec.z;
    return *this;
}

const TVec3& TVec3::operator/=(const TVec3& aVec)
{
    x /= aVec.x;
    y /= aVec.y;
    z /= aVec.z;
    return *this;
}


const TVec3& TVec3::operator*=(float aScalar)
{
    x *= aScalar;
    y *= aScalar;
    z *= aScalar;
    return *this;
}

const TVec3& TVec3::operator/=(float aScalar)
{
    float k = 1.0f / aScalar;
    x *= k;
    y *= k;
    z *= k;
    return *this;
}

const TVec3& TVec3::AtomicAdd(const TVec3& aVec)
{
    #pragma omp atomic
    x += aVec.x;
    #pragma omp atomic
    y += aVec.y;
    #pragma omp atomic
    z += aVec.z;

    return *this;
}

const TVec3 operator+(const TVec3& aVecA, const TVec3& aVecB)
{
    return TVec3(aVecA.x + aVecB.x, aVecA.y + aVecB.y, aVecA.z + aVecB.z);
}

const TVec3 operator+(const TVec3& aVec, float aScalar)
{
    return TVec3(aVec.x + aScalar, aVec.y + aScalar, aVec.z + aScalar);
}

const TVec3 operator-(const TVec3& aVecA, const TVec3& aVecB)
{
    return TVec3(aVecA.x - aVecB.x, aVecA.y - aVecB.y, aVecA.z - aVecB.z);
}

const TVec3 operator-(const TVec3& aVec, float aScalar)
{
    return TVec3(aVec.x - aScalar, aVec.y - aScalar, aVec.z - aScalar);
}

const TVec3 operator*(const TVec3& aVecA, const TVec3& aVecB)
{
    return TVec3(aVecA.x * aVecB.x, aVecA.y * aVecB.y, aVecA.z * aVecB.z);
}

const TVec3 operator*(const TVec3& aVec, float aScalar)
{
    return TVec3(aVec.x * aScalar, aVec.y * aScalar, aVec.z * aScalar);
}

const TVec3 operator*(float aScalar, const TVec3& aVec)
{
    return TVec3(aVec.x * aScalar, aVec.y * aScalar, aVec.z * aScalar);
}

const TVec3 operator/(const TVec3& aVecA, const TVec3& aVecB)
{
    return TVec3(aVecA.x / aVecB.x, aVecA.y / aVecB.y, aVecA.z / aVecB.z);
}

const TVec3 operator/(const TVec3& aVec, float aScalar)
{
    float k = 1.0f / aScalar;
    return TVec3(aVec.x * k, aVec.y * k, aVec.z * k);
}

float length(const TVec3& aVec)
{
    return sqrtf(sqlength(aVec));
}

float sqlength(const TVec3& aVec)
{
    return aVec.x * aVec.x + aVec.y * aVec.y + aVec.z * aVec.z;
}

const TVec3 normalize(const TVec3& aVec)
{
    return aVec / length(aVec);
}

float dot(const TVec3& aVecA, const TVec3& aVecB)
{
    return aVecA.x * aVecB.x + aVecA.y * aVecB.y + aVecA.z * aVecB.z;
}

const TVec3 cross(const TVec3& aVecA, const TVec3& aVecB)
{
    return TVec3( (aVecA.y * aVecB.z - aVecA.z * aVecB.y),
                 -(aVecA.x * aVecB.z - aVecA.z * aVecB.x),
                  (aVecA.x * aVecB.y - aVecA.y * aVecB.x));
}

const TVec3 lerp(const TVec3& aVecA, const TVec3& aVecB, float aParameter)
{
     return (1.f - aParameter) * aVecA + aParameter * aVecB;
}

const TVec3 reflect(const TVec3& aVecV, const TVec3& aVecN)
{
    return aVecV - 2.0f * dot(aVecV, aVecN) * aVecN;
}

bool refract(const TVec3& aVecV, const TVec3& aVecN, float lNiOverNt, TVec3& aOutRefracted)
{
    TVec3 lUnitV = normalize(aVecV);
    float lDotUVN = dot(lUnitV, aVecN);
    float lDiscriminant = 1.0f - lNiOverNt * lNiOverNt * (1.0f - lDotUVN * lDotUVN);
    
    if(lDiscriminant > 0.f)
    {
        aOutRefracted = lNiOverNt * (lUnitV - aVecN * lDotUVN) - aVecN * sqrtf(lDiscriminant);
        return true;
    }

    return false;
}

const TVec3 vmax(const TVec3& aVecA, const TVec3& aVecB)
{
    return TVec3(   fmaxf(aVecA.x, aVecB.x),
                    fmaxf(aVecA.y, aVecB.y),
                    fmaxf(aVecA.z, aVecB.z));
}

const TVec3 vmin(const TVec3& aVecA, const TVec3& aVecB)
{
    return TVec3(   fminf(aVecA.x, aVecB.x),
                    fminf(aVecA.y, aVecB.y),
                    fminf(aVecA.z, aVecB.z));
}

const TVec3 vabs(const TVec3& aVec)
{
    return TVec3(   fabsf(aVec.x),
                    fabsf(aVec.y),
                    fabsf(aVec.z));
}