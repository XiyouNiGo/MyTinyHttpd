#include "mytinyhttpd/net/inet_address.h"

#include <gtest/gtest.h>

#include "mytinyhttpd/base/logging.h"

using namespace mytinyhttpd;
using namespace mytinyhttpd::net;

TEST(InetAddressTest, Inet4AddressTest) {
  InetAddress addr0(1234);
  ASSERT_EQ(addr0.ToIp(), std::string("0.0.0.0"));
  ASSERT_EQ(addr0.ToIpPort(), std::string("0.0.0.0:1234"));
  ASSERT_EQ(addr0.port(), 1234);

  InetAddress addr1(4321, true);
  ASSERT_EQ(addr1.ToIp(), std::string("127.0.0.1"));
  ASSERT_EQ(addr1.ToIpPort(), std::string("127.0.0.1:4321"));
  ASSERT_EQ(addr1.port(), 4321);

  InetAddress addr2("1.2.3.4", 8888);
  ASSERT_EQ(addr2.ToIp(), std::string("1.2.3.4"));
  ASSERT_EQ(addr2.ToIpPort(), std::string("1.2.3.4:8888"));
  ASSERT_EQ(addr2.port(), 8888);

  InetAddress addr3("255.254.253.252", 65535);
  ASSERT_EQ(addr3.ToIp(), std::string("255.254.253.252"));
  ASSERT_EQ(addr3.ToIpPort(), std::string("255.254.253.252:65535"));
  ASSERT_EQ(addr3.port(), 65535);
}

TEST(InetAddressTest, Inet6AddressTest) {
  InetAddress addr0(1234, false, true);
  ASSERT_EQ(addr0.ToIp(), std::string("::"));
  ASSERT_EQ(addr0.ToIpPort(), std::string("[::]:1234"));
  ASSERT_EQ(addr0.port(), 1234);

  InetAddress addr1(1234, true, true);
  ASSERT_EQ(addr1.ToIp(), std::string("::1"));
  ASSERT_EQ(addr1.ToIpPort(), std::string("[::1]:1234"));
  ASSERT_EQ(addr1.port(), 1234);

  InetAddress addr2("2001:db8::1", 8888, true);
  ASSERT_EQ(addr2.ToIp(), std::string("2001:db8::1"));
  ASSERT_EQ(addr2.ToIpPort(), std::string("[2001:db8::1]:8888"));
  ASSERT_EQ(addr2.port(), 8888);

  InetAddress addr3("fe80::1234:abcd:1", 8888);
  ASSERT_EQ(addr3.ToIp(), std::string("fe80::1234:abcd:1"));
  ASSERT_EQ(addr3.ToIpPort(), std::string("[fe80::1234:abcd:1]:8888"));
  ASSERT_EQ(addr3.port(), 8888);
}

TEST(InetAddressTest, InetAddressResolveTest) {
  InetAddress addr;
  if (InetAddress::Resolve("baidu.com", &addr)) {
    LOG_INFO << "baidu.com resolved to " << addr.ToIpPort();
  } else {
    LOG_ERROR << "Unable to resolve baidu.com";
  }
}
