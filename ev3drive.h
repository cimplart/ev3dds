#ifndef EV3DRIVE_H
#define EV3DRIVE_H

#pragma once

#include "ev3nodesubscriber.h"


class Ev3Drive : public Ev3NodeSubscriber::CommandListener
{
public:
    Ev3Drive();
    ~Ev3Drive();

    void on_move_command(const Ev3MoveCommand& cmd) override;

private:

};

#endif