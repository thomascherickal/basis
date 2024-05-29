#include <basis/core/coordinator.h>
#include <basis/core/coordinator_connector.h>

#include <gtest/gtest.h>

struct TestTransportManager : public basis::core::transport::TransportManager {
  TestTransportManager() : TransportManager(nullptr) {
    // Ensure we actually test the coordinator and not just transport manager
    // todo: we can just make two transport managers
    use_local_publishers_for_subscribers = false;
  }

  /**
   * Clear local publisher info, ensuring we only pull data from the coordinator
   */
  void ClearLocalPublisherInfo() {}
};

TEST(TestCoordinator, BasicTest) {
  basis::core::transport::Coordinator coordinator = *basis::core::transport::Coordinator::Create();

  auto connector = basis::core::transport::CoordinatorConnector::Create();
  ASSERT_NE(connector, nullptr);

  basis::core::transport::PublisherInfo pub_info;
  pub_info.publisher_id = basis::core::transport::CreatePublisherId();
  pub_info.topic = "test_topic";
  pub_info.transport_info["net_tcp"] = "1234";

  basis::core::transport::proto::TransportManagerInfo sent_info;
  *sent_info.add_publishers() = pub_info.ToProto();

  connector->SendTransportManagerInfo(sent_info);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  coordinator.Update();
}

struct TestRawStruct {
  uint32_t foo = 3;
  float bar = 8.5;
  char baz[4] = "baz";
};

TEST(TestCoordinator, TestPubSubOrder) {
  using namespace basis::core::threading;

  using namespace basis::core::networking;
  using namespace basis::core::transport;

  using namespace basis::plugins::transport;
  basis::core::transport::Coordinator coordinator = *basis::core::transport::Coordinator::Create();

  auto connector = basis::core::transport::CoordinatorConnector::Create();

  auto thread_pool_manager = std::make_shared<ThreadPoolManager>();
  TestTransportManager transport_manager;
  transport_manager.RegisterTransport("net_tcp", std::make_unique<TcpTransport>(thread_pool_manager));

  std::atomic<int> callback_times{0};
  SubscriberCallback<TestRawStruct> callback = [&](std::shared_ptr<const TestRawStruct> t) {
    spdlog::warn("Got the message {} {} {}", t->foo, t->bar, t->baz);
    callback_times++;
  };

  auto update = [&](int expected_publishers) {
    // local to transport manager
    {
      // let the transport manager gather any new publishers
      transport_manager.Update();
      auto info = transport_manager.GetTransportManagerInfo();
      ASSERT_EQ(expected_publishers, info.publishers_size());
      // send it off to the coordinator
      connector->SendTransportManagerInfo(std::move(info));
      // give the TCP stack time to work
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    {
      // let the coordinator process any new publishers, send out updates
      coordinator.Update();
      // give the TCP stack time to work
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // local to transport manager
    {
      // Let the connector pull the data back from coordinator
      connector->Update();
      //
      if (connector->GetLastNetworkInfo()) {
        transport_manager.HandleNetworkInfo(*connector->GetLastNetworkInfo());
      }

      transport_manager.Update();
    }
  };

  update(0);
  {
    std::shared_ptr<Subscriber<TestRawStruct>> prev_sub =
        transport_manager.Subscribe<TestRawStruct, basis::core::serialization::RawSerializer>("test_struct", callback);

    update(0);
    auto test_publisher =
        transport_manager.Advertise<TestRawStruct, basis::core::serialization::RawSerializer>("test_struct");

    update(1);

    ASSERT_EQ(test_publisher->GetSubscriberCount(), 1);

    {
      std::shared_ptr<Subscriber<TestRawStruct>> after_sub =
          transport_manager.Subscribe<TestRawStruct, basis::core::serialization::RawSerializer>("test_struct",
                                                                                                callback);
      update(1);
      ASSERT_EQ(test_publisher->GetSubscriberCount(), 2);
      auto send_msg = std::make_shared<const TestRawStruct>();
      test_publisher->Publish(send_msg);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      ASSERT_EQ(callback_times, 2);
    }
    update(1);
  }

  // Update and check that we forgot about the publisher
  update(0);
}