#ifndef CNR_IPC_UTILITIES_EXAMPLE_MY_SERVER_CLIENT
#define CNR_IPC_UTILITIES_EXAMPLE_MY_SERVER_CLIENT

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <memory>
#include <utility>
#include <array>

#include <boost/asio.hpp>

#include <cnr_ipc_utilities/asio_utilities.h>

struct get_t
{
  struct Request
  {
    std::string module_id;
    uint16_t index;
    uint8_t subindex;
  } req;

  struct Response
  {
    bool success;
    std::string what;
    std::array<uint8_t, 8> value;
  } res;
};

struct set_t
{
  struct Request
  {
    std::string module_id;
    uint16_t index;
    uint8_t subindex;

    std::array<uint8_t, 8> value;
  } req;

  struct Response
  {
    bool success;
    std::string what;
  } res;
};

template <typename R>
inline std::string to_string(const R& req)
{
  return std::string();
}

template <>
inline std::string to_string(const get_t::Request& req)
{
  Json::Value json;
  json["module_id"] = req.module_id;
  json["index"] = req.index;
  json["subindex"] = req.subindex;
  json.setComment(std::string("// EOF"), Json::CommentPlacement::commentAfter);

  Json::StreamWriterBuilder stream_write_builder;
  return Json::writeString(stream_write_builder, json);
}

template <>
inline std::string to_string(const get_t::Response& res)
{
  Json::Value json;
  json["success"] = res.success;
  json["value"] = Json::Value(Json::arrayValue);
  for (std::size_t i = 0; i != 8; i++)
  {
    Json::Value v = res.value[i];
    json["value"].append(v);
  }
  json["what"] = res.what;
  json.setComment(std::string("// EOF"), Json::CommentPlacement::commentAfter);

  Json::StreamWriterBuilder stream_write_builder;
  return Json::writeString(stream_write_builder, json);
}

template <>
inline std::string to_string(const set_t::Request& req)
{
  Json::Value json;
  json["module_id"] = req.module_id;
  json["index"] = req.index;
  json["subindex"] = req.subindex;
  json["value"] = Json::Value(Json::arrayValue);
  for (std::size_t i = 0; i != 8; i++)
  {
    Json::Value v = req.value[i];
    json["value"].append(v);
  }
  json.setComment(std::string("// EOF"), Json::CommentPlacement::commentAfter);

  Json::StreamWriterBuilder stream_write_builder;
  auto str = Json::writeString(stream_write_builder, json);
  return str;
}

template <>
inline std::string to_string(const set_t::Response& res)
{
  Json::Value json;
  json["success"] = res.success;
  json["what"] = res.what;
  json.setComment(std::string("// EOF"), Json::CommentPlacement::commentAfter);

  Json::StreamWriterBuilder stream_write_builder;
  return Json::writeString(stream_write_builder, json);
}

template <typename R>
inline bool validate(const std::string& income, std::string& what)
{
  Json::CharReaderBuilder char_reader_builder;
  Json::CharReader* reader = char_reader_builder.newCharReader();

  Json::Value json;
  std::string errors;

  bool parsingSuccessful = reader->parse(income.c_str(), income.c_str() + income.size(), &json, &errors);
  delete reader;

  if (!parsingSuccessful)
  {
    what = std::string(__PRETTY_FUNCTION__) + ":" + std::to_string(__LINE__) +
           ": Failed to parse the JSON, errors: " + errors;
    return false;
  }

  bool ok = true;
  std::vector<std::string> mandatory_fields;
  std::string elab = "";
  
  if (typeid(R).name() == typeid(get_t::Request).name())
  {
    elab = "get_t::Request()";
    mandatory_fields = {"module_id", "index", "subindex"};
  }
  else if (typeid(R).name() == typeid(set_t::Request).name())
  {
    elab = "set_t::Request()";
    mandatory_fields = {"module_id", "index", "subindex", "value"};
  }
  if (typeid(R).name() == typeid(get_t::Response).name())
  {
    elab = "get_t::Response()";
    mandatory_fields = {"success", "value"};
  }
  else if (typeid(R).name() == typeid(set_t::Response).name())
  {
    elab = "set_t::Response()";
    mandatory_fields = {"success"};
  }

  what = "";
  std::stringstream ss;
  ss << "******************* VALIDATE:" << elab << std::endl;
  ss << json << std::endl;
  ss << "mandatory fields: ";
  for (auto const& f : mandatory_fields)
  {
    ss << f << ",";
    if (!json.isMember(f))
    {
      what += "'" + f + "' ";
      ok = false;
    }
  }
  if (!ok)
  {
    what = "In the JSON message are missing the fields: " + what + "\n\n'''''\n" +ss.str()+"'''";
  }

  return ok;
}

template <typename R>
inline bool to_json(const std::string& income, Json::Value& ret, std::string& what)
{
  if (!validate<R>(income, what))
  {
    what = "Failed to parse the JSON, errors: " + what;
    return false;
  }

  Json::CharReaderBuilder char_reader_builder;
  Json::CharReader* reader = char_reader_builder.newCharReader();

  std::string errors;

  bool parsingSuccessful = reader->parse(income.c_str(), income.c_str() + income.size(), &ret, &errors);
  delete reader;

  if (!parsingSuccessful)
  {
    what = "Failed to parse the JSON, errors: " + errors;
    return false;
  }
  return true;
}

template <typename R>
inline R from_string(const std::string&)
{
  return R();
}
/**
 * @brief
 *
 * @param req
 * @return std::string
 */
template <>
inline get_t::Request from_string(const std::string& str)
{
  get_t::Request req;

  std::string what;
  Json::Value json;
  if(!to_json<get_t::Request>(str,json,what))
  {
    throw std::runtime_error(what.c_str());
  }

  req.module_id = json.get("module_id", "").asString();
  req.index = json.get("index", 0).asInt();
  req.subindex = json.get("subindex", 0).asInt();
  return req;
}

template <>
inline get_t::Response from_string(const std::string& str)
{
  get_t::Response res;
  std::string what;
  Json::Value json;
  if(!to_json<get_t::Response>(str,json,what))
  {
    throw std::runtime_error(what.c_str());
  }

  res.success = json["success"].asBool();
  res.what = json["what"].asString();
  Json::Value tmp = json["value"];
  for (int i = 0; i < 8; i++) res.value[i] = tmp[i].asUInt();

  return res;
}

template <>
inline set_t::Request from_string(const std::string& str)
{
  set_t::Request req;
  std::string what;
  Json::Value json;
  if(!to_json<set_t::Request>(str,json,what))
  {
    throw std::runtime_error(what.c_str());
  }

  req.module_id = json.get("module_id", "").asString();
  req.index = json.get("index", 0).asUInt();
  req.subindex = json.get("subindex", 0).asUInt();

  for (int i = 0; i < 8; i++) req.value[i] = json["value"][i].asUInt();

  return req;
}

template <>
inline set_t::Response from_string(const std::string& str)
{
  set_t::Response res;
  std::string what;
  Json::Value json;
  if(!to_json<set_t::Response>(str,json,what))
  {
    throw std::runtime_error(what.c_str());
  }

  std::cout << __PRETTY_FUNCTION__ <<"'"<< str <<"'"<< std::endl;
  std::cout << __PRETTY_FUNCTION__ <<"'"<< json  <<"'"<< std::endl;

  res.success = json["success"].asBool();
  res.what = json["what"].asString();

  return res;
}



/**
 * @brief
 *
 */
class MyServer : public std::enable_shared_from_this<MyServer>
{
 public:
  MyServer(
      const std::map<int, std::string>& config, const std::string& host, short set_port, short get_port)
      : config_(config)
  {
    try
    {
      std::cout << "Prepare the SET SDO server .." << std::endl;
      set_server_ = cnr::ipc::make_shared(
          io_context_, host, set_port, std::bind(&MyServer::set, this, std::placeholders::_1, std::placeholders::_2));

      std::cout << "Prepare the GET SDO server .." << std::endl;

      get_server_ = cnr::ipc::make_shared(
          io_context_, host, get_port, std::bind(&MyServer::get, this, std::placeholders::_1, std::placeholders::_2));

      std::cout << "Run the context .." << std::endl;

      io_context_thread_ = std::thread(
          [this]()
          {
            std::cout << "The IO CONTEXT IS READY, RUN!" << std::endl;
            io_context_.run();
            std::cout << "The IO CONTEXT HAS BEEN TERMINATED" << std::endl;
          });
    }
    catch (std::exception& e)
    {
      std::stringstream ss;
      ss << __PRETTY_FUNCTION__ << ": Exception:" << e.what() << std::endl;
      throw std::runtime_error(ss.str().c_str());
    }
  }

  ~MyServer()
  {
    std::cout << "The IO CONTEXT WILL BE STOPPED" << std::endl;
    io_context_.stop();
    std::cout << "Joint the thread..." << std::endl;
    io_context_thread_.join();

    std::cout << "Destroy servers ..." << std::endl;
    set_server_.reset();
    get_server_.reset();
  }

  void set(const std::string& income, std::string& outcome)
  {
    set_t::Response res;
    std::string what = "set";
    try
    {
      set_t::Request req = from_string<set_t::Request>(income);

      std::string what;
      auto result = std::find_if(
          config_.begin(), config_.end(), [req](const auto& mo) { return mo.second == req.module_id; });

      res.success = (result == config_.end()) ? false : true;

      if (!res.success)
      {
        res.what = "Failed set: the module is not in the map";
      }
    }
    catch(const std::exception& e)
    {
      res.success = false; 
      res.what = e.what();
    }
    outcome = to_string(res);
  }

  void get(const std::string& income, std::string& outcome)
  {
    get_t::Response res;
    std::string what;

    try
    {
      get_t::Request req = from_string<get_t::Request>(income);

      std::string what;
      auto result = std::find_if(
          config_.begin(), config_.end(), [req](const auto& mo) { return mo.second == req.module_id; });

      res.success = (result == config_.end()) ? false : true;

      if (!res.success)
      {
        res.what = "Failed set: the module is not in the map";
      }
    }
    catch(const std::exception& e)
    {
      res.success = false; 
      res.what = e.what();
    }
    outcome = to_string(res);
  }

 private:
  const std::map<int, std::string> config_;

  boost::asio::io_context io_context_;
  std::thread io_context_thread_;
  std::shared_ptr<cnr::ipc::AsioServer> set_server_;
  std::shared_ptr<cnr::ipc::AsioServer> get_server_;
};

/**
 * @brief
 *
 */
struct MyClient
{
  cnr::ipc::AsioClient c;

  MyClient(const std::string& host, const std::string& port)
  {
    try
    {
      c.connect(host, port, std::chrono::seconds(10));
    }
    catch (std::exception& e)
    {
      std::cerr << __PRETTY_FUNCTION__ << ":" << __LINE__ << ": Client Connection Error \n";
      std::cerr << __PRETTY_FUNCTION__ << ":" << __LINE__ << ": Host: " << host << ", port: " << port << "\n";
      std::cerr << __PRETTY_FUNCTION__ << ":" << __LINE__ << ": Exception: " << e.what() << "\n";
    }
  }

  template <typename R>
  void write_correct_request()
  {
    R req;
    req.index = 1;
    req.module_id = "elmo1";
    req.index = 0x1234;
    req.subindex = 1;
    std::string req_str = to_string(req);
    std::error_code error = c.write_request(req_str, std::chrono::seconds(2));
    if (error)
    {
      std::cerr << error.message() << std::endl;
    }
  }

  template <typename R>
  void write_incorrect_request_1()
  {
    R req;
    req.module_id = "ciao";
    req.index = 0x1234;
    req.subindex = 1;
    std::string req_str = to_string(req);
    c.write_request(req_str, std::chrono::seconds(10));
  }

  void write_incorrect_request_2()
  {
    Json::Value write_root;
    write_root["what"] = "SET";
    write_root["address"] = 0x123456;
    Json::StreamWriterBuilder stream_write_builder;
    const std::string json_file = Json::writeString(stream_write_builder, write_root);
    c.write_request(json_file, std::chrono::seconds(10));
  }

  template <typename R>
  bool read_response()
  {
    std::string response = c.read_response(std::chrono::seconds(10));
    R res = from_string<R>(response);
    std::cout << "Success: " << res.success << std::endl;
    return res.success;
  }
};

#endif  // CNR_IPC_UTILITIES_EXAMPLE_MY_SERVER_CLIENT
