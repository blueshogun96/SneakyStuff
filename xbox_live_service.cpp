#include "pch.h"
#include "xbox_live_service.h"


using namespace Windows::UI::Popups;


void dialog_box(const wchar_t* text)
{
	Platform::String^ str = ref new Platform::String(text);

	MessageDialog Dialog(str, "Test");
	Dialog.ShowAsync();

	OutputDebugStringW(str->Data());
	OutputDebugStringW(L"\n");
}

bool xbox_live_service_t::sign_in_silently()
{
	try
	{
		user = std::make_shared<xbox::services::system::xbox_live_user>();
		if (user != nullptr)
		{
			auto op = user->signin_silently(nullptr);
			create_task(op).then([this](xbox::services::xbox_live_result<xbox::services::system::sign_in_result> result)
			{
				if (!result.err())
				{
					switch (result.payload().status())
					{
					case xbox::services::system::sign_in_status::success:
						login_status = 1;
						login_errors = L"Success";
						break;

					case xbox::services::system::sign_in_status::user_cancel:
						login_status = 0;
						login_errors = L"Cancelled";
						break;

					case xbox::services::system::sign_in_status::user_interaction_required:
						login_status = -1;
						login_errors = L"UX required...";
						break;
					}
				}
				else
				{
					login_status = -1;
					wchar_t chr_val[200];
					_itow_s(result.err().value(), chr_val, 200, 16);
					login_errors = std::wstring(chr_val);
				}

				dialog_box(login_errors.c_str());
				if (login_status == -1)
					sign_in_via_ux();
				else if (login_status == 1)
				{
					context = std::make_shared<xbox::services::xbox_live_context>(user);
					sign_out_handler();
				}
			});
		}
		else
			return false;
	}
	catch (std::exception ex)
	{
		errors = ex.what();
	}

	return true;
}

bool xbox_live_service_t::sign_in_via_ux()
{
	try
	{
		if (login_status == -1)
			login_status = -2;

		Concurrency::create_task( user->signin( Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher))
			.then([this](xbox::services::xbox_live_result<xbox::services::system::sign_in_result> result)
		{
			if (!result.err())
			{
				switch (result.payload().status())
				{
				case xbox::services::system::sign_in_status::success:
					login_status = 1;
					login_errors = L"Success";
					break;
				case xbox::services::system::sign_in_status::user_cancel:
					login_status = 0;
					login_errors = L"Cancelled";
					break;
				default:
					break;
				}
			}
			else
			{
				login_status = -3;
				wchar_t chr_val[200];
				_itow_s(result.err().value(), chr_val, 200, 16);
				login_errors = std::wstring(chr_val);
			}

			dialog_box(login_errors.c_str());

			if (login_status == 1)
			{
				context = std::make_shared<xbox::services::xbox_live_context>(user);
				sign_out_handler();
			}
		});
	}
	catch (std::exception ex)
	{
		errors = ex.what();
	}

	return true;
}

void xbox_live_service_t::sign_out_handler()
{
	xbox::services::system::xbox_live_user::add_sign_out_completed_handler(
		[this](const xbox::services::system::sign_out_completed_event_args& args)
	{
		user = nullptr;
		context = nullptr;
	});
}

std::wstring xbox_live_service_t::get_gamertag()
{
	try
	{
		if( login_status == 1 && user != nullptr )
		{
			return user->gamertag();
		}
	}
	catch (std::exception ex)
	{
		errors = ex.what();
	}
}

/*
 * References:
 * https://docs.microsoft.com/en-us/gaming/xbox-live/using-xbox-live/auth/authentication-for-UWP-projects
 * https://social.technet.microsoft.com/wiki/contents/articles/51027.xbox-live-authentication-for-uwp-projects-using-c.aspx
 */