/*
 *  Software License Agreement (New BSD License)
 *
 *  Copyright 2020 National Council of Research of Italy (CNR)
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the copyright holder(s) nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <map>

#include "../examples/my_server_client.h"

namespace detail
{
struct unwrapper
{
  explicit unwrapper(std::exception_ptr pe) : pe_(pe) {}

  operator bool() const { return bool(pe_); }

  friend auto operator<<(std::ostream& os, unwrapper const& u) -> std::ostream&
  {
    try
    {
      std::rethrow_exception(u.pe_);
      return os << "no exception";
    }
    catch (std::runtime_error const& e)
    {
      return os << "runtime_error: " << e.what();
    }
    catch (std::logic_error const& e)
    {
      return os << "logic_error: " << e.what();
    }
    catch (std::exception const& e)
    {
      return os << "exception: " << e.what();
    }
    catch (...)
    {
      return os << "non-standard exception";
    }
  }
  std::exception_ptr pe_;
};
}  // namespace detail

auto unwrap(std::exception_ptr pe) { return detail::unwrapper(pe); }

template <class F>
::testing::AssertionResult does_not_throw(F&& f)
{
  try
  {
    f();
    return ::testing::AssertionSuccess();
  }
  catch (...)
  {
    return ::testing::AssertionFailure() << unwrap(std::current_exception());
  }
}
std::shared_ptr<MyClient> get_client;
std::shared_ptr<MyClient> set_client;
std::shared_ptr<MyServer> server;

constexpr unsigned short set_srv_port = 1024;
constexpr unsigned short get_srv_port = 1025;

TEST(TestSuite, srvCreation)
{
  std::map<int, std::string> config{{1, "elmo1"}, {2, "elmo2"}, {3, "elmo3"}};
  EXPECT_TRUE(does_not_throw(
      [&]
      {
        server.reset(
            new MyServer(config, "localhost", set_srv_port, get_srv_port));
      }));
}

TEST(TestSuite, clntSetCreation)
{
  EXPECT_TRUE(
      does_not_throw([&] { set_client.reset(new MyClient("localhost", std::to_string(set_srv_port))); }));
}

TEST(TestSuite, clntGetCreation)
{
  EXPECT_TRUE(
      does_not_throw([&] { get_client.reset(new MyClient("localhost", std::to_string(get_srv_port))); }));
}

TEST(TestSuite, clntSet)
{
  EXPECT_TRUE(bool(server));
  EXPECT_TRUE(does_not_throw([&] { set_client->write_correct_request<set_t::Request>(); }));
  EXPECT_TRUE(set_client->read_response<set_t::Response>());
}

TEST(TestSuite, clntSetErr1)
{
  EXPECT_TRUE(bool(server));
  
  EXPECT_TRUE(does_not_throw([&] { set_client->write_incorrect_request_1<set_t::Request>(); }));
  usleep(10000);
  EXPECT_FALSE(set_client->read_response<set_t::Response>());
  
}

TEST(TestSuite, clntSetErr2)
{
  EXPECT_TRUE(bool(server));
  EXPECT_TRUE(does_not_throw([&] { set_client->write_incorrect_request_2(); }));
  EXPECT_FALSE(set_client->read_response<set_t::Response>());
}


TEST(TestSuite, clntGet)
{
  EXPECT_TRUE(server);
  EXPECT_TRUE(does_not_throw([&] { get_client->write_correct_request<get_t::Request>(); }));
  EXPECT_TRUE(get_client->read_response<get_t::Response>());
}

TEST(TestSuite, clntGetErr)
{
  EXPECT_TRUE(does_not_throw([&] { get_client->write_incorrect_request_1<set_t::Request>(); }));
  EXPECT_FALSE(get_client->read_response<get_t::Response>());
  EXPECT_TRUE(does_not_throw([&] { get_client->write_incorrect_request_2(); }));
  EXPECT_FALSE(get_client->read_response<get_t::Response>());
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
