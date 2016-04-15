/*
  Copyright (c) 2014-2016 DataStax

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/
#ifndef __INTEGRATION_HPP__
#define __INTEGRATION_HPP__

#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>

#include "cassandra.h"

#include "bridge.hpp"
#include "logger.hpp"
#include "objects.hpp"
#include "values.hpp"

#if defined(_WIN32) && defined(_DEBUG) && !defined(USE_VISUAL_LEAK_DETECTOR)
// Enable memory leak detection
# define _CRTDBG_MAP_ALLOC
# include <stdlib.h>
# include <crtdbg.h>

// Enable memory leak detection for new operator
# ifndef DBG_NEW
#   define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#   define new DBG_NEW
# endif
#endif

#ifdef _WIN32
# define PATH_SEPARATOR "\\"
#else
# define PATH_SEPARATOR "/"
#endif

// Create simple console logging functions
#define PREFIX_LOG std::cout
#define PREFIX_MESSAGE "Integration Tests: "
#define SUFFIX_LOG std::endl
#ifdef INTEGRATION_VERBOSE_LOGGING
# define LOG(message) PREFIX_LOG << PREFIX_MESSAGE << message << SUFFIX_LOG
# define LOG_WARN(message) PREFIX_LOG << PREFIX_MESSAGE << "WARN: " << message << SUFFIX_LOG
#else
# define LOG_DISABLED do {} while (false)
# define LOG(message) LOG_DISABLED
# define LOG_WARN(message) LOG_DISABLED
#endif
#define LOG_ERROR(message) PREFIX_LOG << PREFIX_MESSAGE << "ERROR: " \
                           << __FILE__ << "(" << __LINE__ << "): " \
                           << message << SUFFIX_LOG

#define SKIP_TEST(message) \
  std::cout << "[ SKIPPED  ] " << message << std::endl; \
  return;

#define CHECK_FAILURE \
  if (HasFailure()) { \
    return; \
  }

#define CHECK_VERSION(version) \
  if (server_version_ < #version) { \
    SKIP_TEST("Unsupported for server version " \
              << server_version_.to_string()) \
  }

#define CHECK_CONTINUE(flag, message) \
  ASSERT_TRUE(flag) << message; \

#define SELECT_ALL_SYSTEM_LOCAL_CQL "SELECT * FROM system.local"

using namespace driver;
using namespace driver::object;

/**
 * Statement type enumeration to use for specifying type of statement to use
 * when executing queries
 */
enum StatementType {
  /**
   * Batch statement
   */
  STATEMENT_TYPE_BATCH,
  /**
   * Prepared statement
   */
  STATEMENT_TYPE_PREPARED,
  /**
   * Simple statement
   */
  STATEMENT_TYPE_SIMPLE
};

/**
 * Base class to provide common integration test functionality
 */
class Integration : public testing::Test {
public:

  Integration();

  virtual ~Integration();

  virtual void SetUp();

  virtual void TearDown();

protected:
  /**
   * Handle for interacting with CCM
   */
  SmartPtr<CCM::Bridge> ccm_;
  /**
   * Logger instance for handling driver log messages
   */
  driver::Logger logger_;
  /**
   * Cluster instance
   */
  Cluster cluster_;
  /**
   * Connected database session
   */
  shared::SessionPtr session_;
  /**
   * Generated keyspace name for the integration test
   */
  std::string keyspace_name_;
  /**
   * Generated table name for the integration test
   */
  std::string table_name_;
  /**
   * UUID generator
   */
  shared::UuidGenPtr uuid_generator_;
  /**
   * Version of Cassandra/DSE the session is connected to
   */
  CCM::CassVersion server_version_;
  /**
   * Number of nodes in data center one
   * (DEFAULT: 1)
   */
  unsigned short number_dc1_nodes_;
  /**
   * Number of nodes in data center two
   * (DEFAULT: 0)
   */
  unsigned short number_dc2_nodes_;
  /**
   * Replication factor override; default is calculated based on number of data
   * center nodes; single data center is (nodes / 2) rounded up
   */
  unsigned short replication_factor_;
  /**
   * Default contact points generated based on the number of nodes requested
   */
  std::string contact_points_;
  /**
   * Setting for client authentication. True if client authentication should be
   * enabled; false otherwise.
   * (DEFAULT: false)
   */
  bool is_client_authentication_;
  /**
   * Setting for SSL authentication. True if SSL should be enabled; false
   * otherwise.
   * (DEFAULT: false)
   */
  bool is_ssl_;
  /**
   * Setting for schema metadata. True if schema metadata should be enabled;
   * false otherwise.
   * (DEFAULT: false)
   */
  bool is_schema_metadata_;
  /**
   * Setting to determine if CCM cluster should be started. True if CCM cluster
   * should be started; false otherwise.
   * (DEFAULT: true)
   */
  bool is_ccm_start_requested_;
  /**
   * Setting to determine if session connection should be established. True if
   * session connection should be established; false otherwise.
   * (DEFAULT: true)
   */
  bool is_session_requested_;
  /**
   * Name of the test case/suite
   */
  std::string test_case_name_;
  /**
   * Name of the test
   */
  std::string test_name_;

  /**
   * Create the cluster configuration and establish the session connection using
   * provided cluster object.
   *
   * @param cluster Cluster object to use when creating session connection
   */
  void connect(Cluster cluster);

  /**
   * Generate the contact points for the cluster
   *
   * @param ip_address IP address prefix
   * @param number_of_nodes Total number of nodes in the cluster
   * @return Comma delimited IP address (e.g. contact points)
   */
  std::string generate_contact_points(const std::string& ip_prefix,
      size_t number_of_nodes) const;

  /**
   * Variable argument string formatter
   *
   * @param format Format string that follows printf specifications
   * @param ... Additional arguments; depends on the format string
   */
  std::string format_string(const char* format, ...) const;

  /**
   * Convert a string to lowercase
   *
   * @param input String to convert to lowercase
   */
  std::string to_lower(const std::string& input);

  /**
   * Concatenate an array/vector into a string
   *
   * @param elements Array/Vector elements to concatenate
   * @param delimiter Character to use between elements (default: <space>)
   * @return A string representation of all the array/vector elements
   */
  std::string implode(const std::vector<std::string>& elements,
      const char delimiter = ' ') const;

  /**
   * Split a string into an array/vector
   *
   * @param input String to convert to array/vector
   * @param delimiter Character to use split into elements (default: <space>)
   * @return An array/vector representation of the string
   */
  std::vector<std::string> explode(const std::string& input,
      const char delimiter = ' ');

  /**
   * Replace all occurrences of a string from the input string
   *
   * @param input String having occurrences being replaced
   * @param from String to find for replacement
   * @param to String to replace with
   * @return Input string with replacement
   */
  std::string replace_all(const std::string& input,
      const std::string& from, const std::string& to) const;

private:
  /**
  * Keyspace creation query (generated via SetUp)
  */
  std::string create_keyspace_query_;

// TODO: Properly implement these (and others) as utilities
public:
  /**
   * Cross platform millisecond granularity sleep
   *
   * @param milliseconds Time in milliseconds to sleep
   */
  static void msleep(unsigned int milliseconds);

  /**
   * Remove the leading and trailing whitespace from a string
   *
   * @param input String to trim
   * @return Trimmed string
   */
  static std::string trim(const std::string& input);

  /**
   * Get the current working directory
   *
   * @return Current working directory
   */
  static std::string cwd();

  /**
   * Check to see if a file exists
   *
   * @param filename Absolute/Relative filename
   * @return True if file exists; false otherwise
   */
  static bool file_exists(const std::string& filename);

  /**
   * Create the directory from a path
   *
   * @param path Directory/Path to create
   */
  static void mkdir(const std::string& path);
};

#endif //__INTEGRATION_HPP__
