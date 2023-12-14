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
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
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
    static constexpr eprosima::fastdds::dds::DomainId_t myDomain = 0;
	eprosima::fastdds::dds::DomainParticipant *myParticipant = nullptr;
	eprosima::fastdds::dds::Publisher *myPublisher = nullptr;
    eprosima::fastdds::dds::Topic *mySensorEventTopic = nullptr;
    eprosima::fastdds::dds::DataWriter *myWriter = nullptr;

    eprosima::fastdds::dds::TypeSupport mySensorEventType;
    eprosima::fastdds::dds::TypeSupport myButtonEventType;

    using DomainParticipant = eprosima::fastdds::dds::DomainParticipant;

    class MyDomainListener : public eprosima::fastdds::dds::DomainParticipantListener
    {
        virtual void on_participant_discovery(DomainParticipant* /*participant*/,
            eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override
        {
            if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)  {
                std::cout << "New participant discovered" << std::endl;
            } else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT ||
                    info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)  {
                std::cout << "New participant lost" << std::endl;
            }
        }        

        virtual void on_subscriber_discovery(
                DomainParticipant* /*participant*/,
                eprosima::fastrtps::rtps::ReaderDiscoveryInfo&& info) override
        {
            if (info.status == eprosima::fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERED_READER)  {
                std::cout << "New subscriber discovered" << std::endl;
            } else if (info.status == eprosima::fastrtps::rtps::ReaderDiscoveryInfo::REMOVED_READER)   {
                std::cout << "New subscriber lost" << std::endl;
            }
        }        
    };

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