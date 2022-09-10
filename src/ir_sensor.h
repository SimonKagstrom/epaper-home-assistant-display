#pragma once

#include <memory>
#include <functional>

class IIRSensor
{
public:
    class ICookie
    {
    public:
        virtual ~ICookie() = default;
    };
    virtual ~IIRSensor() = default;

    /**
     * @brief get movement callbacks until the returned value is released
     *
     * @param onMovement callback
     * @return a cookie
     */
    virtual std::unique_ptr<ICookie> listenToMotion(const std::function<void()> onMovement) = 0;

    virtual bool hasMotion() const = 0;

    static std::unique_ptr<IIRSensor> create();
};
