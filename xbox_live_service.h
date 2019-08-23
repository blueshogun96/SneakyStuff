#pragma once

#define XSAPI_CPP 1
#define _NO_XSAPIIMP
#define XBOX_LIVE_CREATORS_SDK
#include <xsapi/services.h>

class xbox_live_service_t
{
public:
	xbox_live_service_t() 
	{
		gamertag = L"(No User)";
		login_errors = L"Starting...";
		user = nullptr;
		context = nullptr;
		login_status = 0;
	}

	bool sign_in_silently();
	bool sign_in_via_ux();
	
	std::wstring get_gamertag();

	int get_login_status() const { return login_status; }

protected:
	void sign_out_handler();

	std::wstring gamertag;
	int login_status;
	std::shared_ptr<xbox::services::system::xbox_live_user> user;
	std::shared_ptr<xbox::services::xbox_live_context> context;
	std::wstring login_errors;
	std::string errors;
};

