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

#ifndef EV3NODEPUBLISHER_H
#define EV3NODEPUBLISHER_H

#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <memory>
#include "ev3dataPubSubTypes.h"

class Ev3NodePublisher
{
public:
    Ev3NodePublisher();
    virtual ~Ev3NodePublisher();

    /**
     * Read sensors and publish their events.
     */
    void process_sensors();

private:
    static constexpr eprosima::fastdds::dds::DomainId_t myDomain = 1;
	eprosima::fastdds::dds::DomainParticipant *myParticipant = nullptr;
	eprosima::fastdds::dds::Publisher *myPublisher = nullptr;
    eprosima::fastdds::dds::Topic *mySensorEventTopic = nullptr;
    eprosima::fastdds::dds::DataWriter *myWriter = nullptr;

    eprosima::fastdds::dds::TypeSupport mySensorEventType;
    eprosima::fastdds::dds::TypeSupport myButtonEventType;

    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        PubListener()
        //    : matched_(0)
        //    , firstConnected_(false)
        {
        }

        ~PubListener() override
        {
        }

        // void on_publication_matched(
        //         eprosima::fastdds::dds::DataWriter* writer,
        //         const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        //int matched_;

        //bool firstConnected_;
    } myListener;

};

#endif //EV3NODEPUBLISHER_H