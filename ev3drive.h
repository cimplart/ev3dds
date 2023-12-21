#ifndef EV3DRIVE_H
#define EV3DRIVE_H

#pragma once

#include "ev3nodesubscriber.h"
#include "ev3dev.h"

class Ev3Drive : public Ev3NodeSubscriber::CommandListener
{
public:
    Ev3Drive();
    ~Ev3Drive();

    void on_move_command(const Ev3MoveCommand& cmd) override;

private:

    static constexpr int SPEED = 500;

    ev3dev::large_motor myLeftMotor;
    ev3dev::large_motor myRightMotor;

    void drive(int leftSpeed, int rightSpeed);
    void stop();

};

#endif