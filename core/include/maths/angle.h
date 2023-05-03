#pragma once
#include <numbers>
#include <cmath>

namespace core
{

inline constexpr static float PI = std::numbers::pi_v<float>;
class Degree;
/**
 * \brief Radian is an utility class that describes radian angles (0 to 2PI).
 * It can be easily converted to Degree with conversion constructor.
 * It can be used with trigonometric functions (Sin, Cos, Tan)
 */
class Radian
{
public:
    constexpr Radian() = default;
    constexpr explicit Radian(float value) : value_(value) {}
    /**
     * \brief Conversion constructor that implicitly converts Degree to Radian
     * \param angle is the degree angle to be converted to Radian
     */
    constexpr Radian(const Degree& angle);
    [[nodiscard]] constexpr float value() const { return value_; }

    constexpr Radian operator+(Radian angle) const { return Radian{ value_ + angle.value() }; }
    constexpr Radian& operator+=(Radian angle)
    {
        value_ += angle.value();
        return *this;
    }
    constexpr Radian operator-(Radian angle) const { return Radian{ value_ - angle.value() }; }
    constexpr Radian& operator-=(Radian angle)
    {
        value_ -= angle.value();
        return *this;
    }
    constexpr Radian operator*(float value) const { return Radian{ value_ * value }; }
    constexpr Radian operator/(float value) const { return Radian{ value_ / value }; }
    constexpr Radian operator-() const { return Radian{ -value_ }; }
private:
    float value_ = 0.0f;
};

/**
 * \brief Degree is an utility class that describes degree angles (0 to 360).
 * It can be easily converted to Radian with conversion constructor.
 * It can be used with trigonometric functions (Sin, Cos, Tan)
 */
class Degree
{
public:
    constexpr Degree() = default;
    constexpr explicit Degree(float value) : value_(value) {}
    /**
     * \brief Conversion constructor that implicitly converts Radian to Degree
     * \param angle is the radian angle to be converted to Degree
     */
    constexpr Degree(const Radian& angle) : value_(angle.value() / PI * 180.0f) {}
    [[nodiscard]] constexpr float value() const { return value_; }
    constexpr Degree operator+(Degree angle) const { return Degree{ value_ + angle.value() }; }
    constexpr Degree& operator+=(Degree angle)
    {
        value_ += angle.value();
        return *this;
    }
    constexpr Degree operator-(Degree angle) const { return Degree{ value_ - angle.value() }; }
    constexpr Degree& operator-=(Degree angle)
    {
        value_ -= angle.value();
        return *this;
    }
    constexpr Degree operator*(float value) const { return Degree{ value_ * value }; }
    constexpr Degree operator/(float value) const { return Degree{ value_ / value }; }
    constexpr Degree operator-() const { return Degree{ -value_ }; }
private:
    float value_ = 0.0f;
};

constexpr Degree operator*(float value, Degree angle) { return Degree{ angle.value() * value }; }


constexpr Radian::Radian(const Degree& angle)
{
    value_ = angle.value() / 180.0f * PI;
}

/**
 * \brief Sin is a function that calculates the sinus of a given angle.
 * \param angle is the given angle
 * \return the result of the sinus of angle
 */
inline float Sin(Radian angle)
{
    return std::sin(angle.value());
}

/**
 * \brief Cos is a function that calculates the cosinus of a given angle.
 * \param angle is the given angle
 * \return the result of the cosinus of angle
 */
inline float Cos(Radian angle)
{
    return std::cos(angle.value());
}

/**
 * \brief Tan is a function that calculates the tangent of a given angle.
 * \param angle is the given angle
 * \return the result of the tangent of angle
 */
inline float Tan(Radian angle)
{
    return std::tan(angle.value());
}

/**
 * \brief Asin is a function that calculates the angle of a given ratio.
 * \param ratio is the given ratio between the opponent and hypothenuse
 * \return the result of the asinus function
 */
inline Radian Asin(float ratio)
{
    return Radian{ std::asin(ratio) };
}

/**
 * \brief Acos is a function that calculates the angle of a given ratio.
 * \param ratio is the given ratio between the adjacent and hypothenuse
 * \return the result of the acosinus function
 */
inline Radian Acos(float ratio)
{
    return Radian{ std::acos(ratio) };
}

/**
 * \brief Atan is a function that calculates the angle of a given ratio.
 * \param ratio is the given ratio between the adjacent and hypothenuse
 * \return the result of the atan function
 */
inline Radian Atan(float ratio)
{
    return Radian{ std::atan(ratio) };
}

/**
 * \brief Atan2 is a function that calculates the angle of a given ratio between two parameters.
 * \param y is the upper value of the ratio
 * \param x is the lower value of the ratio
 * \return the result of the atan function
 */
inline Radian Atan2(float y, float x)
{
    return Radian{ std::atan2(y,x) };
}
} // namespace core
