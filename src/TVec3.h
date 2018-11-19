
#ifndef RT_TVEC3_H
#define RT_TVEC3_H

struct TVec3
{
    union
    {
        struct
        {
            float x, y, z;
        };
        float mValues[3];
    };

    TVec3(): x(0.f), y(0.f), z(0.f){;}
    explicit TVec3(float aValue);
    TVec3(float aX, float aY, float aZ);

    const TVec3& operator+() const { return *this; }
    const TVec3 operator-() const { return TVec3(-x, -y, -z); };

    const TVec3& operator+=(const TVec3& aVec);
    const TVec3& operator+=(float aScalar);
    const TVec3& operator-=(const TVec3& aVec);
    const TVec3& operator-=(float aScalar);
    const TVec3& operator*=(const TVec3& aVec);
    const TVec3& operator/=(const TVec3& aVec);

    const TVec3& TVec3::operator*=(float aScalar);
    const TVec3& TVec3::operator/=(float aScalar);

    const TVec3& AtomicAdd(const TVec3& aVec);

    float operator[](size_t aIndex) const { return mValues[aIndex]; }
};

const TVec3 operator+(const TVec3& aVecA, const TVec3& aVecB);
const TVec3 operator+(const TVec3& aVecA, float aScalara);
const TVec3 operator-(const TVec3& aVecA, const TVec3& aVecB);
const TVec3 operator-(const TVec3& aVecA, float aScalar);
const TVec3 operator*(const TVec3& aVecA, const TVec3& aVecB);
const TVec3 operator*(const TVec3& aVecA, float aScalar);
const TVec3 operator*(float aScalar, const TVec3& aVecB);
const TVec3 operator/(const TVec3& aVecA, const TVec3& aVecB);
const TVec3 operator/(const TVec3& aVecA, float aScalar);

float length(const TVec3& aVec);
float sqlength(const TVec3& aVec);
const TVec3 normalize(const TVec3& aVec);
float dot(const TVec3& aVecA, const TVec3& aVecB);
const TVec3 cross(const TVec3& aVecA, const TVec3& aVecB);
const TVec3 lerp(const TVec3& aVecA, const TVec3& aVecB, float aParameter);
const TVec3 reflect(const TVec3& aVecV, const TVec3& aVecN);
bool refract(const TVec3& aVecV, const TVec3& aVecN, float lNiOverNt, TVec3& aOutRefracted);
const TVec3 vmax(const TVec3& aVecA, const TVec3& aVecB);
const TVec3 vmin(const TVec3& aVecA, const TVec3& aVecB);
const TVec3 vabs(const TVec3& aVec);
/*
template <typename T> const T lerp(const T& aValueA, const T& aValueB, float aParameter)
{
    return (1.f - aParameter) * aValueA + aParameter * aValueB;
}
*/

typedef TVec3 vec3;
#endif //RT_TVEC3_H