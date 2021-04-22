extern "C"
{
#include "coldclear.h" 
}
#include "shak.hpp"
#include <chrono>
#include <iostream>

#include <string>
#include <thread>
#ifdef _WIN32
#include "libusb.h"
#include <Windows.h>
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#include <libusb-1.0/libusb.h>
#endif

	extern libusb_context *ctx;
	extern libusb_device_handle *dev_handle;
	extern libusb_device **devs;
	extern bool libusbinit;
	extern ssize_t cnt;
	extern int r;
	enum
	{
		MAXQUEUE = 6
	};
	// https://github.com/mat1jaczyyy/MisaMinoNET/blob/master/MisaMinoNET/MisaMino/main.cpp

	//typedef void(configure)(Misomino_AI_Param, bool, bool, bool, int);
	//typedef void(update_next)(const char*);
	//typedef void(update_current)(const char*);
	//typedef void(update_hold)(const char*);
	//typedef void(update_incoming)(int);
	//typedef void(update_combo)(int);
	//typedef void(update_b2b)(int);
	//typedef void(update_field)(const char*);
	//typedef void(update_reset)();
	//typedef void(action)(char*, int);
	//typedef bool(alive)();
	//typedef void(findpath)(const char*, const char*, int, int, int, bool, char*, int);
	//
	//typedef CCAsyncBot* (cc_launch_async)(CCOptions* options, CCWeights* weights);
	//typedef CCAsyncBot* (cc_launch_with_board_async)(CCOptions* options, CCWeights* weights, bool* field,
	//	uint32_t bag_remain, CCPiece* hold, bool b2b, uint32_t combo);
	//typedef void(cc_reset_async)(CCAsyncBot* bot, bool* field, bool b2b, uint32_t combo);
	//typedef void(cc_add_next_piece_async)(CCAsyncBot* bot, CCPiece piece);
	//typedef void(cc_request_next_move)(CCAsyncBot* bot, uint32_t incoming);
	//typedef CCBotPollStatus(cc_poll_next_move)(CCAsyncBot* bot, CCMove* move, CCPlanPlacement* plan, uint32_t* plan_length);
	//typedef CCBotPollStatus(cc_block_next_move)(CCAsyncBot* bot, CCMove* move, CCPlanPlacement* plan, uint32_t* plan_length);
	//typedef void(cc_default_options)(CCOptions* options);
	//typedef void(cc_default_weights)(CCWeights* weights);
	//typedef void(cc_fast_weights)(CCWeights* weights);
	//
	//std::string GetLastDllError()
	//{
	//#ifdef _WIN32
	//	// Get the error message, if any.
	//	DWORD errorMessageID = GetLastError();
	//	if (errorMessageID == 0)
	//		return std::string(); // No error message has been recorded
	//
	//	LPSTR messageBuffer = nullptr;
	//	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
	//
	//	std::string message(messageBuffer, size);
	//
	//	// Free the buffer.
	//	LocalFree(messageBuffer);
	//
	//	return message;
	//#endif
	//#if defined(__linux__) || defined(__APPLE__)
	//	return std::string(dlerror());
	//#endif
	//}
	//
	//#ifdef _WIN32
	//template <typename T>
	//T* GetDllFunction(HMODULE handle, std::string name)
	//{
	//	return (T*)GetProcAddress(handle, name.c_str());
	//}
	//#endif
	//#if defined(__linux__) || defined(__APPLE__)
	//template <typename T>
	//T* GetDllFunction(void* handle, std::string name)
	//{
	//	return (T*)dlsym(handle, name.c_str());
	//}
	//#endif

	const int ccpiece[7] = {CC_S, CC_Z, CC_J, CC_L, CC_T, CC_O, CC_I};

	void printdev(libusb_device *dev)
	{
		libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0)
		{
			std::cout << "failed to get device descriptor" << std::endl;
			return;
		}
		std::cout << "Number of possible configurations: " << (int)desc.bNumConfigurations << "  ";
		std::cout << "Device Class: " << (int)desc.bDeviceClass << "  ";
		std::cout << "VendorID: " << desc.idVendor << "  ";
		std::cout << "ProductID: " << desc.idProduct << std::endl;
		libusb_config_descriptor *config;
		libusb_get_config_descriptor(dev, 0, &config);
		std::cout << "Interfaces: " << (int)config->bNumInterfaces << " ||| ";
		const libusb_interface *inter;
		const libusb_interface_descriptor *interdesc;
		const libusb_endpoint_descriptor *epdesc;
		for (int i = 0; i < (int)config->bNumInterfaces; i++)
		{
			inter = &config->interface[i];
			std::cout << "Number of alternate settings: " << inter->num_altsetting << std::endl;
			for (int j = 0; j < inter->num_altsetting; j++)
			{
				interdesc = &inter->altsetting[j];
				std::cout << "Interface Number: " << (int)interdesc->bInterfaceNumber << std::endl;
				std::cout << "Number of endpoints: " << (int)interdesc->bNumEndpoints << std::endl;
				for (int k = 0; k < (int)interdesc->bNumEndpoints; k++)
				{
					epdesc = &interdesc->endpoint[k];
					std::cout << "Descriptor Type: " << (int)epdesc->bDescriptorType << std::endl;
					std::cout << "EP Address: " << (int)epdesc->bEndpointAddress << std::endl;
				}
			}
		}
		std::cout << std::endl
				  << std::endl
				  << std::endl;
		libusb_free_config_descriptor(config);
	}

	CCPiece ppt_to_cc_piece(uint8_t nintendo)
	{
		return (CCPiece)ccpiece[nintendo];
	}
	enum
	{
		get_entire_queue = 1,
		get_last_que = 2,
		press_controllerinput = 3,
		wait_until_can_move = 4,
		wait_frame = 5,
		release_controllerinput = 6
	};

	void switch_get_entire_queue(USBResponse x)
	{ //1
		int actual;
		int r;

		unsigned char ee = get_entire_queue;
		r = libusb_bulk_transfer(dev_handle, Switch::switchsend, &ee, 1, &actual, 0);

		r = libusb_bulk_transfer(dev_handle, Switch::switchread, (unsigned char *)x.data, 5, &actual, 0);
	}
	void switch_get_last_que(USBResponse x)
	{ //2
		int actual;
		int r;

		unsigned char ee = get_last_que;
		r = libusb_bulk_transfer(dev_handle, Switch::switchsend, &ee, 1, &actual, 0);

		r = libusb_bulk_transfer(dev_handle, Switch::switchread, (unsigned char *)x.data[6], 1, &actual, 0);
	}
	void switch_press_controllerinput(PcToSwitchCntrlr input)
	{ //3
		int actual;
		int r;

		unsigned char ee = press_controllerinput;
		r = libusb_bulk_transfer(dev_handle, Switch::switchsend, &ee, 1, &actual, 0);
		ee = input;
		r = libusb_bulk_transfer(dev_handle, Switch::switchsend, &ee, 1, &actual, 0);
	}
	void switch_release_controllerinput(PcToSwitchCntrlr input)
	{ // 6
		int actual;
		int r;

		unsigned char ee = release_controllerinput;
		r = libusb_bulk_transfer(dev_handle, Switch::switchsend, &ee, 1, &actual, 0);

		ee = input;
		r = libusb_bulk_transfer(dev_handle, Switch::switchsend, &ee, 1, &actual, 0);
	}
	void switch_wait_until_can_move()
	{ //4
		int actual;
		int r;

		unsigned char ee = wait_until_can_move;
		r = libusb_bulk_transfer(dev_handle, Switch::switchsend, &ee, 1, &actual, 0);

		ee = 'f';
		r = libusb_bulk_transfer(dev_handle, Switch::switchread, &ee, 1, &actual, 0);
	}

	void switch_wait_frame()
	{ //5
		int actual;
		int r;

		unsigned char ee = wait_frame;
		r = libusb_bulk_transfer(dev_handle, Switch::switchsend, &ee, 1, &actual, 0);
	}

	int main()
	{
		initlibusb();

		// std::string path = "cold_clear.dll";
		// getbytes(x);
		// int joe = 0x69;
		// std::cout << "size: " << std::to_string(x.size) << std::endl
		// 		  << "joe: " << joe <<" compared to "<< x.data;

		// {libusb_device **devss;		//pointer to pointer of device, used to retrieve a list of devices
		// libusb_context *ctxx = NULL; //a libusb session
		// int rr;						//for return values
		// ssize_t cnt;				//holding number of devices in list
		// rr = libusb_init(&ctxx);		//initialize a library session
		// if (rr < 0)
		// {
		// 	std::cout << "Init Error " << rr << std::endl; //there was an error
		// 	 return 1;
		// }
		// libusb_set_option(ctxx, LIBUSB_OPTION_USE_USBDK); //set verbosity level to 3, as suggested in the documentation
		// cnt = libusb_get_device_list(ctxx, &devss); //get the list of devices
		// if (cnt < 0)
		// {
		// 	std::cout << "Get Device Error" << std::endl; //there was an error
		// }
		// std::cout << cnt << " Devices in list." << std::endl; //print total number of usb devices
		// ssize_t i;									//for iterating through the list
		// for (i = 1; i < cnt; i++)
		// {
		// 	 printdev(devss[i]); //print specs of this device
		// }
		// libusb_free_device_list(devss, 1); //free the list, unref the devices in it
		// libusb_exit(ctxx);				  //close the session
		// }

		// using namespace std;
		// libusb_device **devs;			  //pointer to pointer of device, used to retrieve a list of devices
		// libusb_device_handle *dev_handle; //a device handle
		// libusb_context *ctx = NULL;		  //a libusb session
		// int r;							  //for return values
		// ssize_t cnt;					  //holding number of devices in list
		// r = libusb_init(&ctx);			  //initialize the library for the session we just declared
		// if (r < 0)
		// {
		// 	cout << "Init Error " << r << endl; //there was an error
		// 	return 1;
		// }
		// //libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation
		// libusb_set_option(ctx, LIBUSB_OPTION_USE_USBDK);
		// cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
		// if (cnt < 0)
		// {
		// 	cout << "Get Device Error" << endl; //there was an error
		// 	return 1;
		// }
		// cout << cnt << " Devices in list." << endl;
		// dev_handle = libusb_open_device_with_vid_pid(ctx, 0x057e, 0x3000); //these are vendorID and productID I found for my usb device
		// if (dev_handle == NULL)
		// 	cout << "Cannot open device" << endl;
		// else
		// 	cout << "Device Opened" << endl;
		// libusb_free_device_list(devs, 1);			//free the list, unref the devices in it
		// unsigned char *data = new unsigned char[4]; //data to write
		// data[0] = 'a';
		// data[1] = 'b';
		// data[2] = 'c';
		// data[3] = 'd'; //some dummy values
		// int actual;	   //used to find out how many bytes were written
		// if (libusb_kernel_driver_active(dev_handle, 0) == 1)
		// { //find out if kernel driver is attached
		// 	cout << "Kernel Driver Active" << endl;
		// 	if (libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
		// 		cout
		// 			<< "Kernel Driver Detached!" << endl;
		// }
		// r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)
		// if (r < 0)
		// {
		// 	cout << "Cannot Claim Interface" << endl;
		// 	return 1;
		// }
		// cout << "Claimed Interface" << endl;
		// cout
		// 	<< "Data->" << data << "<-" << endl; //just to see the data we want to write : abcd
		// cout << "reading Data..." << endl;
		// unsigned char ee = 'e';
		// int bruh = 21;
		// for (int i = 1; i < 0xfff; i++){
		// 	r = libusb_bulk_transfer(dev_handle, 5, (unsigned char *)&bruh, 4, &bruh, 100); //my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
		// cout << "received:" << to_string(bruh) << "on" << to_string(i) << endl;}
		// if (r == 0 && actual == 8) //we wrote the 4 bytes successfully
		// 	cout << "read Successful " << endl;
		// else
		// 	cout << "Write Error:" << std::to_string(r) << endl;
		// r = libusb_release_interface(dev_handle, 0); //release the claimed interface
		// if (r != 0)
		// {
		// 	cout << "Cannot Release Interface" << endl;
		// 	return 1;
		// }
		// cout << "Released Interface" << endl;
		// libusb_close(dev_handle); //close the device we opened
		// libusb_exit(ctx);		  //needs to be called to end the
		// delete[] data;			  //delete the allocated memory for data
		// return 0;

		// initlibusb();
		// int b = 0;
		// int bruh = 0;
		// libusb_bulk_transfer(dev_handle, Switch::switchread, (unsigned char *)&bruh, 4, &b, 0);
		// std::cout << "before: " << " b: " << b << " bruh: " << bruh << std::endl;
		// // for (int i = 1; i < 0xfff; i++)
		// // {
		// // 	b, bruh = 0;
		// libusb_bulk_transfer(dev_handle, Switch::switchread, (unsigned char *)&b, bruh, &bruh, 0);
		// std::cout << "after: " << " b: " << b << " bruh: " << bruh << std::endl;
		// // }
		//getbytes(x);

		USBResponse x = {0};
		x.data = new uint8_t[400];
		CCWeights weights;
		CCOptions options;
		CCMove move;
		CCPlanPlacement placement;
		uint32_t plan_length = 1;
		CCBotPollStatus staus;
		cc_default_options(&options);
		cc_default_weights(&weights);
		CCAsyncBot *bot; // =  cc_launch_async(&options, &weights);

		switch_get_entire_queue(x);

		std::cout << "got qeueue" << std::endl;
		for (int i = 0; i < MAXQUEUE; i++)
		{
			cc_add_next_piece_async(bot, ppt_to_cc_piece(x.data[i]));
		};

		std::cout << "pieces: " << x.data << std::endl;
		int movement_count = 0;
		bool not_holding = true;
		int sleep_current = 0;
		int sleep_start = 0;
		bool first_hold = true;
		bool pressed_DLEFT = false;
		bool pressed_DRIGHT = false;
		bool pressed_DUP = false;
		bool pressed_A = false;
		bool pressed_B = false;
		bool pressed_L = false;
		bool pressed_DDOWN = false;

		while (true)
		{

			std::cout << "getting switch last queue" << std::endl;
			switch_get_last_que(x);
			cc_request_next_move(bot, 0);
			staus = cc_block_next_move(bot, &move, &placement, &plan_length);

			if (staus == CC_BOT_DEAD)
			{
				std::cout << "this is so sad, can we hit 50 kids (coldclear didnt start)" << std::endl;
				break;
			}

			movement_count = 0;
			not_holding = true;
			sleep_current;
			sleep_start;
			first_hold = true;
			pressed_DLEFT = false;
			pressed_DRIGHT = false;
			pressed_DUP = false;
			pressed_A = false;
			pressed_B = false;
			pressed_L = false;
			pressed_DDOWN = false;

			std::cout << "start movement" << std::endl;
			while (movement_count <= move.movement_count)
			{

				std::cout << "wait a frame" << std::endl;
				switch_wait_frame();

				pressed_DLEFT = false;
				pressed_DRIGHT = false;
				pressed_DUP = false;
				pressed_A = false;
				pressed_B = false;
				pressed_L = false;
				pressed_DDOWN = false;

				if (not_holding && move.hold)
				{
					switch_press_controllerinput(switch_L); //L PRESS HOLD YOU DIPSHIT make sure this is added omfg
					pressed_L = true;
					not_holding = false;

					switch_wait_until_can_move();

					if (first_hold)
					{
						first_hold = false;
						cc_add_next_piece_async(bot, ppt_to_cc_piece(MAXQUEUE));
					}
				}
				//CONTROLLER SHIT
				if (move.movements[movement_count] == CC_CW)
				{
					switch_press_controllerinput(switch_A); //A
					pressed_A = true;
				}
				else if (move.movements[movement_count] == CC_CCW)
				{
					switch_press_controllerinput(switch_B); //B
					pressed_B = true;
				}
				else if (move.movements[movement_count] == CC_LEFT)
				{
					switch_press_controllerinput(switch_DLEFT); //DLEFT
					pressed_DLEFT = true;
				}
				else if (move.movements[movement_count] == CC_RIGHT)
				{
					switch_press_controllerinput(switch_DRIGHT); //RIGHT
					pressed_DRIGHT = true;
				}
				else if (move.movements[movement_count] == CC_DROP)
				{
					if (movement_count == move.movement_count)
					{
						switch_press_controllerinput(switch_DUP); //DUP / HARDDROP
						pressed_DUP = true;
					}
					else
					{
						switch_press_controllerinput(switch_DDOWN); //DDOWN / SOFTDROP
						pressed_DDOWN = true;
						//recieve when on floor
					}
				}
				switch_wait_frame();
				if (pressed_DLEFT)
				{
					switch_release_controllerinput(switch_DLEFT); // DLEFT
				}
				if (pressed_DRIGHT)
				{
					switch_release_controllerinput(switch_DRIGHT); // DRIGHT
				}
				if (pressed_DUP)
				{
					switch_release_controllerinput(switch_DUP); // DUP
					switch_wait_until_can_move();
				}
				if (pressed_A)
				{
					switch_release_controllerinput(switch_A); // A
				}
				if (pressed_B)
				{
					switch_release_controllerinput(switch_B); // B
				}
				if (pressed_L)
				{
					switch_release_controllerinput(switch_L); // L
				}
				if (pressed_DDOWN)
				{
					switch_release_controllerinput(switch_DDOWN); // DDOWN
				}

				//wait a frame

				movement_count++;

				if (placement.cleared_lines[0] > -1)
				{
					std::cout << "Line Clear :0" << std::endl;
					switch_wait_until_can_move();
				}
			}
		}

		uninitlibusb();
		return 0;
	}

	// dobytes(x);
	// std::cout << "size:" << x.size << "data:" << x.data;
	// uninitlibusb();
	// SerializeProtocol serializeProtocol;

	//#ifdef _WIN32
	//HMODULE sharedLibHandle = LoadLibraryEx(path.c_str(), NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	//#endif
	//#if defined(__linux__) || defined(__APPLE__)
	//void* sharedLibHandle = (void*)dlopen(path.c_str(), RTLD_LAZY);
	//#endif
	//
	//if (!sharedLibHandle)
	//{
	//	std::cout << "Shared library error: " << GetLastDllError() << std::endl;
	//	return 1;
	//}
	//
	//
	// configure *configure_function = GetDllFunction<configure>(sharedLibHandle, "configure");
	//update_next* update_next_function = GetDllFunction<update_next>(sharedLibHandle, "update_next");
	//
	//cc_launch_async* cc_launch_async_function = GetDllFunction<cc_launch_async>(sharedLibHandle, "cc_launch_async");
	//cc_launch_with_board_async* cc_launch_with_board_async_function = GetDllFunction<cc_launch_with_board_async>(sharedLibHandle, "cc_launch_with_board_async");
	//cc_add_next_piece_async* cc_add_next_piece_async_function = GetDllFunction<cc_add_next_piece_async>(sharedLibHandle, "cc_add_next_piece_async");
	//cc_request_next_move* cc_request_next_move_function = GetDllFunction<cc_request_next_move>(sharedLibHandle, "cc_request_next_move");
	//cc_poll_next_move* cc_poll_next_move_function = GetDllFunction<cc_poll_next_move>(sharedLibHandle, "cc_poll_next_move");
	//cc_block_next_move* cc_block_next_move_function = GetDllFunction<cc_block_next_move>(sharedLibHandle, "cc_block_next_move");
	//cc_default_options* cc_default_options_function = GetDllFunction<cc_default_options>(sharedLibHandle, "cc_default_options");
	//cc_default_weights* cc_default_weights_function = GetDllFunction<cc_default_weights>(sharedLibHandle, "cc_default_weights");
	//cc_fast_weights* cc_fast_weights_function = GetDllFunction<cc_fast_weights>(sharedLibHandle, "cc_fast_weights");
	//
	// update_current *update_current_function = GetDllFunction<update_current>(sharedLibHandle, "update_current");
	// update_hold *update_hold_function = GetDllFunction<update_hold>(sharedLibHandle, "update_hold");
	// update_incoming *update_incoming_function = GetDllFunction<update_incoming>(sharedLibHandle, "update_incoming");
	// update_combo *update_combo_function = GetDllFunction<update_combo>(sharedLibHandle, "update_combo");
	// update_b2b *update_b2b_function = GetDllFunction<update_b2b>(sharedLibHandle, "update_b2b");
	// update_field *update_field_function = GetDllFunction<update_field>(sharedLibHandle, "update_field");
	//update_reset *update_reset_function = GetDllFunction<update_reset>(sharedLibHandle, "update_reset");
	// action *action_function = GetDllFunction<action>(sharedLibHandle, "action");
	// alive *alive_function = GetDllFunction<alive>(sharedLibHandle, "alive");
	// findpath *findpath_function = GetDllFunction<findpath>(sharedLibHandle, "findpath");
	// 	libusb_context *ctx = NULL;
	// 	libusb_device_handle *dev_handle;
	// 	int r;
	// 	libusb_device **devs;
	// 	r = libusb_init(&ctx);
	// 	if (r < 0)
	// 	{
	// 		std::cout << "Libusb init Error " << libusb_strerror((libusb_error)r) << std::endl; // there was an error
	// 		return 1;
	// 	}
	// 	r = libusb_set_option(ctx, LIBUSB_OPTION_USE_USBDK);
	// 	ssize_t cnt = libusb_get_device_list(ctx, &devs);
	// 	if (cnt < 0)
	// 	{
	// 		std::cout << "Get Device Error" << std::endl; //there was an error
	// 	}
	// 	std::cout << cnt << " Devices in list." << std::endl; //print total number of usb devices
	// 	ssize_t i;											  //for iterating through the list
	// 	for (i = 1; i < cnt; i++)
	// 	{
	// 		printdev(devs[i]); //print specs of this device
	// 	}
	// 	libusb_free_device_list(devs, 1); //free the list, unref the devices in it
	// 	libusb_exit(ctx);				  //close the session
	// 	return 0;
	// 	if (r < 0)
	// 	{
	// 		std::cout << "Libusb option set Error " << libusb_strerror((libusb_error)r) << std::endl; // there was an error
	// 		return 1;
	// 	}
	// 	while (true)
	// 	{ // loop for trying to connect to switch
	// 		dev_handle = libusb_open_device_with_vid_pid(ctx, 0x057E, 0x3000);
	// 		if (dev_handle != NULL)
	// 			break; //connected!
	// 		std::cout << "Waiting for Switch to connect over USB" << std::endl;
	// 		std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // a second
	// 	}
	// 	libusb_claim_interface(dev_handle, 0);
	// 	// HOW TO SEND LITERALLY ANYTHING
	// 	// send u8 of id of what you are sending
	// 	// send size of data in an int/u32
	// 	// send the serialization of what you are trying to send I think maybe, not sure
	// 	// reep rewards of what you are trying to do, and if you want the path of your next piece, uhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
	//	libusb_endpoint_descriptor e;
	// 	while (true)
	// 	{ //loop for after connected to switch
	// 		uint32_t it;
	// 		MisominoId id = MisominoId::configure;
	// 		r = libusb_bulk_transfer(dev_handle, LIBUSB_ENDPOINT_OUT, (uint8_t *)&it, sizeof(it), (int *)4, 0); //sending to update the board and everything, but the ID first, which is a u8
	// 		if (r == 0)
	// 		{ // r == 1 maybe means it didnt time out probably?
	// 			uint32_t dataSize = 0;
	// 			r = libusb_bulk_transfer(dev_handle, LIBUSB_ENDPOINT_IN, (uint8_t *)&dataSize, sizeof(dataSize), NULL, INT32_MAX); // reading that the size is of next thing thats going to be sent
	// 			if (r == 0) //r == 0 means it fucking worked what? this is untested, oh right
	// 			{			// check if timeout again/something dumb
	// 				if (dataSize != 0)
	// 				{ // sent wrong datasize :)
	// 					uint8_t data[dataSize];
	// 					r = libusb_bulk_transfer(dev_handle, LIBUSB_ENDPOINT_IN, data, dataSize, (int *)dataSize, INT32_MAX);
	// 					if (r == 0)
	// 					{
	// 						switch (id)
	// 						{
	// 						case MisominoId::configure:
	// 						{
	// 							MisminoSend::configure configureStruct;
	// 							serializeProtocol.binaryToData(configureStruct, data, dataSize);
	// 							configure_function(configureStruct.param, configureStruct.holdAllowed, configureStruct.allSpin, configureStruct.TSDonly, configureStruct.search_width);
	// 						}
	// 						break;
	// 						case MisominoId::update_next:
	// 						{
	// 							MisminoSend::update_next update_nextStruct;
	// 							serializeProtocol.binaryToData(update_nextStruct, data, dataSize);
	//                          update_next_function(update_nextStruct.queue.c_str());
	// 						}
	// 						break;
	// 						case MisominoId::update_current:
	// 						{
	// 							MisminoSend::update_current update_currentStruct;
	// 							serializeProtocol.binaryToData(update_currentStruct, data, dataSize);
	// 							update_current_function(update_currentStruct.piece.c_str());
	// 						}
	// 						break;
	// 						case MisominoId::update_hold:
	// 						{
	// 							MisminoSend::update_hold update_holdStruct;
	// 							serializeProtocol.binaryToData(update_holdStruct, data, dataSize);
	// 							update_hold_function(update_holdStruct.piece.c_str());
	// 						}
	// 						break;
	// 						case MisominoId::update_incoming:
	// 						{
	// 							MisminoSend::update_incoming update_incomingStruct;
	// 							serializeProtocol.binaryToData(update_incomingStruct, data, dataSize);
	// 							update_incoming_function(update_incomingStruct.attack);
	// 						}
	// 						break;
	// 						case MisominoId::update_combo:
	// 						{
	// 							MisminoSend::update_combo update_comboStruct;
	// 							serializeProtocol.binaryToData(update_comboStruct, data, dataSize);
	// 							update_combo_function(update_comboStruct.combo);
	// 						}
	// 						break;
	// 						case MisominoId::update_b2b:
	// 						{
	// 							MisminoSend::update_b2b update_b2bStruct;
	// 							serializeProtocol.binaryToData(update_b2bStruct, data, dataSize);
	// 							update_b2b_function(update_b2bStruct.b2b);
	// 						}
	// 						break;
	// 						case MisominoId::update_field:
	// 						{
	// 							MisminoSend::update_field update_fieldStruct;
	// 							serializeProtocol.binaryToData(update_fieldStruct, data, dataSize);
	// 							update_field_function(update_fieldStruct.field.c_str());
	// 						}
	// 						break;
	// 						case MisominoId::findpath:
	// 						{
	// 							MisminoSend::findpath findpathStruct;
	// 							serializeProtocol.binaryToData(findpathStruct, data, dataSize);
	// 							MisminoReturn::findpath findpathRetStruct;
	// 							char str[1024];
	// 							findpath_function(findpathStruct._field.c_str(), findpathStruct._piece.c_str(), findpathStruct.x, findpathStruct.y, findpathStruct.r, findpathStruct.hold, str, sizeof(str));
	// 							findpathRetStruct.str = std::string(str, sizeof(str));
	// 							r = libusb_bulk_transfer(dev_handle, LIBUSB_ENDPOINT_OUT, (uint8_t *)&id, sizeof(id), NULL, INT32_MAX);
	// 							if (r != 0)
	// 							{
	// 								std::cout << "LibUSB error1: " << libusb_error_name(r) << std::endl;
	// 								return 1;
	// 							}
	// 							uint8_t *data;
	// 							uint32_t dataSendSize;
	// 							serializeProtocol.dataToBinary(findpathRetStruct, &data, &dataSendSize);
	// 							r = libusb_bulk_transfer(dev_handle, LIBUSB_ENDPOINT_OUT, (uint8_t *)&dataSendSize, sizeof(dataSendSize), NULL, INT32_MAX);
	// 							if (r != 0)
	// 							{
	// 								std::cout << "LibUSB error2: " << libusb_error_name(r) << std::endl;
	// 								return 1;
	// 							}
	// 							r = libusb_bulk_transfer(dev_handle, LIBUSB_ENDPOINT_OUT, data, dataSendSize, NULL, INT32_MAX);
	// 							if (r != 0)
	// 							{
	// 								std::cout << "LibUSB error3: " << libusb_error_name(r) << std::endl;
	// 								return 1;
	// 							}
	// 							free(data);
	// 						}
	// 						break;
	// 						}
	// 					}
	// 					else
	// 					{
	// 						std::cout << "LibUSB error4: " << libusb_error_name(r) << std::endl;
	// 						return 1;
	// 					}
	// 				}
	// 				else
	// 				{
	// 					switch (id)
	// 					{
	// 					case MisominoId::update_reset:
	// 					{
	//						update_reset_function();
	// 					}
	// 					break;
	// 					case MisominoId::action:
	// 					{
	// 						MisminoReturn::action actionStruct;
	// 						char str[1024];
	// 						action_function(str, sizeof(str));
	// 						actionStruct.str = std::string(str, sizeof(str));
	// 						r = libusb_bulk_transfer(dev_handle, LIBUSB_ENDPOINT_OUT, (uint8_t *)&id, sizeof(id), NULL, INT32_MAX);
	// 						if (r != 0)
	// 						{
	// 							std::cout << "LibUSB error5: " << libusb_error_name(r) << std::endl;
	// 							return 1;
	// 						}
	// 						uint8_t *data;
	// 						uint32_t dataSize;
	//
	// 						serializeProtocol.dataToBinary(actionStruct, &data, &dataSize);
	//
	// 						r = libusb_bulk_transfer(dev_handle, LIBUSB_ENDPOINT_OUT, (uint8_t *)&dataSize, sizeof(dataSize), NULL, INT32_MAX);
	//
	// 						if (r != 0)
	// 						{
	// 							std::cout << "LibUSB error6: " << libusb_error_name(r) << std::endl;
	//
	// 							return 1;
	// 						}
	//
	// 						r = libusb_bulk_transfer(dev_handle, LIBUSB_ENDPOINT_OUT, data, dataSize, NULL, INT32_MAX);
	//
	// 						if (r != 0)
	// 						{
	// 							std::cout << "LibUSB error7: " << libusb_error_name(r) << std::endl;
	//
	// 							return 1;
	// 						}
	//
	// 						free(data);
	// 					}
	// 					break;
	//
	// 					case MisominoId::alive:
	// 					{
	// 						MisminoReturn::alive aliveStruct;
	// 						aliveStruct.alive = alive_function();
	//
	// 						r = libusb_bulk_transfer(dev_handle, LIBUSB_ENDPOINT_OUT, (uint8_t *)&id, sizeof(id), NULL, INT32_MAX);
	//
	// 						if (r != 0)
	// 						{
	// 							std::cout << "LibUSB error8: " << libusb_error_name(r) << std::endl;
	//
	// 							return 1;
	// 						}
	//
	// 						uint8_t *data;
	// 						uint32_t dataSize;
	//
	// 						r = libusb_bulk_transfer(dev_handle, LIBUSB_ENDPOINT_OUT, (uint8_t *)&dataSize, sizeof(dataSize), NULL, INT32_MAX);
	//
	// 						if (r != 0)
	// 						{
	// 							std::cout << "LibUSB error9: " << libusb_error_name(r) << std::endl;
	//
	// 							return 1;
	// 						}
	//
	// 						serializeProtocol.dataToBinary(aliveStruct, &data, &dataSize);
	//
	// 						r = libusb_bulk_transfer(dev_handle, LIBUSB_ENDPOINT_OUT, data, dataSize, NULL, INT32_MAX);
	//
	// 						if (r != 0)
	// 						{
	// 							std::cout << "LibUSB error10: " << libusb_error_name(r) << std::endl;
	//
	// 							return 1;
	// 						}
	//
	// 						free(data);
	// 					}
	// 					break;
	// 					}
	// 				}
	// 			}
	// 			else
	// 			{
	// 				std::cout << "LibUSB error11: " << libusb_error_name(r) << std::endl;
	// 				std::cout << "how did you wait this long wtf" << std::endl; // this should only happen is sending data borked i think, otherwise we wait like a billion ms, which is a long time
	//
	// 				return 1;
	// 			}
	// 		}
	// 		else
	// 		{
	// 			std::cout << "LibUSB error12: " << libusb_error_name(r) << std::endl;
	// 			std::cout << "how did you wait this long wtf" << std::endl; // this should only happen is sending data borked i think, otherwise we wait like a billion ms, which is a long time
	//
	// 			return 1;
	// 		}
	// 	}
	//
	// 	libusb_exit(ctx);
	//
	// 	return 0;
	// }
