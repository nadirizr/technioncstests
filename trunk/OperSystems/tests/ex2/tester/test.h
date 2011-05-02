#ifndef TEST_H_
#define TEST_H_

#include <iostream>
#include <sstream>
#include <string>

extern std::string location; // Used for finding the origin of an exception

#define FAIL(message) do { \
		std::ostringstream os; \
		os << __LINE__; \
		location = std::string(__FILE__) + ":" + os.str() + " " + message; \
		return false; \
	} while (0)

#define ASSERT_TRUE(command) do  { \
    if (!(command)) { \
      FAIL("expected to get true"); \
    } \
  } while (0)
#define ASSERT_FALSE(command) do  { \
    if ((command)) { \
      FAIL("expected to get false"); \
    } \
  } while (0)
#define ASSERT_EQUALS(expected,actual) do { \
		if(!((expected) == (actual))) { \
			FAIL("values not equal"); \
		} \
	} while (0)
#define ASSERT_NO_THROW(command) do {\
		try { \
			(command); \
		} catch (std::exception& e) { \
			FAIL("caught "+e.what()); \
		} catch (...) { \
			FAIL("caught unknown object"); \
		} \
	} while (0)
#define ASSERT_THROW(exception_type,command) do {\
		try { \
			(command); \
			FAIL("missing exception"); \
		} catch (exception_type&) { \
			\
		} catch (std::exception& e) { \
			FAIL("caught "+e.what()); \
		} catch (...) { \
			FAIL("caught unknown object"); \
		} \
	} while (0)
#define RUN_TEST(b) do {\
		try { \
			std::cerr << #b << " ... "; \
			if (b()) std::cerr << "OK" << std::endl; \
			else std::cerr << "Failed (" << location << ")" << std::endl; \
		} catch (...) { \
			std::cerr << "Failed (" << location << ")" << std::endl; \
		} \
	} while (0)
#define RUN_TEST_N_TIMES(b,n) do {\
    int fails = 0; \
		std::cerr << #b << " ... "; \
    for (int i = 0; i < n; ++i) { \
	  	try { \
		  	if (!(b())) ++fails; \
		  } catch (...) { \
        ++fails; \
		  } \
    } \
    if (fails) std::cerr << fails << "/" << n << " Failed" << std::endl; \
    else std::cerr << "OK" << std::endl; \
	} while (0)
#define RUN_SETUP(c) do {\
		try { \
			if (!c) return false; \
		} catch (...) { \
			return false; \
		} \
	} while (0)

#endif /* TEST_H_ */
