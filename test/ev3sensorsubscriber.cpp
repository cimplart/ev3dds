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

#include "ev3sensorsubscriber.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

using namespace eprosima::fastdds::dds;

Ev3SensorSubscriber::Ev3SensorSubscriber() : myType(new Ev3SensorEventPubSubType()) /* object managed by TypeSupport class */
{
    DomainParticipantQos pqos;
    pqos.name("Participant_sub");
    myParticipant = DomainParticipantFactory::get_instance()->create_participant(myDomain, pqos);
    if (myParticipant == nullptr) {
        throw std::runtime_error("Failed creating domain participant");
    }  

    mySubscriber = myParticipant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (mySubscriber == nullptr) {
        DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
        throw std::runtime_error("Failed creating domain subscriber");
    }

    mySensorEventTopic = myParticipant->create_topic("ev3_sensor_event_topic", "Ev3SensorEvent", TOPIC_QOS_DEFAULT);    
    if (mySensorEventTopic == nullptr)  {
        myParticipant->delete_subscriber(mySubscriber);
        DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
        throw std::runtime_error("Failed creating EV3 sensor event topic");
    }

    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    myReader = mySubscriber->create_datareader(mySensorEventTopic, rqos, &myListener);
    if (myReader == nullptr) {
        myParticipant->delete_topic(mySensorEventTopic);
        myParticipant->delete_subscriber(mySubscriber);
        DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
        throw std::runtime_error("Failed creating EV3 data reader");
    }
}

Ev3SensorSubscriber::~Ev3SensorSubscriber()
{
    assert(mySubscriber != nullptr);
    mySubscriber->delete_datareader(myReader);
    assert(myParticipant != nullptr);
    myParticipant->delete_subscriber(mySubscriber);
    myParticipant->delete_topic(mySensorEventTopic);
    DomainParticipantFactory::get_instance()->delete_participant(myParticipant);
}

void Ev3SensorSubscriber::SubListener::on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1) {
        matched_ = info.total_count;
        std::cout << "Subscriber matched." << std::endl;
    } else if (info.current_count_change == -1) {
        matched_ = info.total_count;
        std::cout << "Subscriber unmatched." << std::endl;
    } else {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

static std::map<enum Ev3Button, std::string> buttonStr = {
    { Back, "Back" },
    { Up, "Up" },
    { Down, "Down" },
    { Left, "Left" },
    { Right, "Right" },
    { Enter, "Enter" }
};

void Ev3SensorSubscriber::SubListener::on_data_available(DataReader* reader)
{
    SampleInfo info;
    Ev3ButtonEvent event;
    if (reader->take_next_sample(&event, &info) == ReturnCode_t::RETCODE_OK) {
        if (info.instance_state == ALIVE_INSTANCE_STATE) {
            samples_++;
            std::cout << "Button " << buttonStr[event.button()] << " is " << (event.pressed() ? "pressed" : "released") <<  std::endl;
        }
    }
}
