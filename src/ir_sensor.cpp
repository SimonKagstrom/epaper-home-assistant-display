#include "ir_sensor.h"

#include "wiringPi.h"

namespace
{

class IRSensor : public IIRSensor
{
public:
    class Cookie : public IIRSensor::ICookie
    {
    public:
        explicit Cookie(std::function<void()> onDelete) :
            m_onDelete(onDelete)
        {
        }

        ~Cookie()
        {
            m_onDelete();
        }

    private:
        std::function<void()> m_onDelete;
    };

    IRSensor()
    {
        wiringPiSetupGpio();
        pinMode(g_motion_pin, INPUT);
        wiringPiISR(g_motion_pin, INT_EDGE_BOTH, IRSensor::staticOnMotion);
        g_singleton = this;
    }

    IRSensor(const IRSensor&) = delete;

    ~IRSensor() final
    {
        g_singleton = nullptr;
    }

    bool hasMotion() const final
    {
        return digitalRead(g_motion_pin);
    }

    std::unique_ptr<IIRSensor::ICookie> listenToMotion(const std::function<void()> onMovement) final
    {
        m_movementListener = onMovement;
        return std::unique_ptr<IIRSensor::ICookie>(new Cookie([this]()
        {
            m_movementListener = [](){};
        }));
    }


private:
    void onMotion(void)
    {
        m_movementListener();
    }

    static void staticOnMotion(void)
    {
        if (g_singleton)
        {
            printf("MOTION!\n");
            g_singleton->onMotion();
        }
    }

    const int g_motion_pin = 26;
    std::function<void()> m_movementListener{[](){}};

    static IRSensor *g_singleton;
};

IRSensor *IRSensor::g_singleton;
}


std::unique_ptr<IIRSensor> IIRSensor::create()
{
    return std::make_unique<IRSensor>();
}
