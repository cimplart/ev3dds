// MIT License

// Copyright (c) 2023 Artur Wisz

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef EV3SENSORSUBSCRIBER_H
#define EV3SENSORSUBSCRIBER_H

#include "ev3dataPubSubTypes.h"
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>

class Ev3SensorSubscriber
{
public:
    Ev3SensorSubscriber();
    virtual ~Ev3SensorSubscriber();

private:

    static constexpr eprosima::fastdds::dds::DomainId_t myDomain = 0;
    eprosima::fastdds::dds::DomainParticipant* myParticipant = nullptr;
    eprosima::fastdds::dds::Subscriber* mySubscriber = nullptr;
    eprosima::fastdds::dds::Topic* mySensorEventTopic = nullptr;
    eprosima::fastdds::dds::DataReader* myReader = nullptr;
    eprosima::fastdds::dds::TypeSupport mySensorEventType;
    eprosima::fastdds::dds::TypeSupport myButtonEventType;

    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:
        SubListener()  : matched_(0), samples_(0)
        {
        }

        ~SubListener() override
        {
        }

        void on_data_available(eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(eprosima::fastdds::dds::DataReader* reader,
                                     const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        int matched_;
        uint32_t samples_;
    }  myListener;

    void cleanup_dds_objects();
};

#endif //EV3SENSORSUBSCRIBER_H