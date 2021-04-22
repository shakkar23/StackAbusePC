#include <chrono>
#include <iostream>
#ifdef _WIN32
extern "C"
{
#include "libusb.h"
}
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <libusb-1.0/libusb.h>
#endif
//#include <stdio>
#include "shak.hpp"
#include <iomanip>
#include <string>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#endif

libusb_context *ctx = nullptr;
libusb_device_handle *dev_handle;
libusb_device **devs;
bool libusbinit = false;
ssize_t cnt;
int r;
int interfaceNumber;

Switch readNX()
{
	switch (interfaceNumber)
	{
	case 1:
		return switchread1;
		break;

	case 0:
		return switchread0;
		break;
	default:
		return switchread2;
		break;
	}
	return switchread2;
}

Switch sendNX()
{
	switch (interfaceNumber)
	{
	case 1:
		return switchsend1;
		break;

	case 0:
		return switchsend0;
		break;
	default:
		return switchsend2;
	}
}

bool initlibusb()
{
	if (libusbinit)
	{
		return false;
	}
	using namespace std;

	// pointer to pointer of device, used to retrieve a list of devices
	//libusb_device_handle *dev_handle; // a device handle
	//libusb_context *ctx = NULL;		  // a libusb session
	// for return values
	// holding number of devices in list
	r = libusb_init(&ctx); // initialize the library for the session we just declared
	if (r < 0)
	{
		cout << "Init Error " << r << endl; // there was an error
		return false;
	}
	r = libusb_set_option(ctx, LIBUSB_OPTION_USE_USBDK); // set verbosity level to 3, as suggested in the documentation
	// if (r < 0)
	// {
	// 	cout << "set Error " << r << endl << libusb_error_name(r); // there was an error
	// 	return;
	// }
	cnt = libusb_get_device_list(ctx, &devs); // get the list of devices
	if (cnt < 0)
	{
		std::cout << "Get Device Error" << std::endl; // there was an error
		return false;
	}
	std::cout << cnt << " Devices in list." << std::endl;

	dev_handle = libusb_open_device_with_vid_pid(ctx, 0x057e, 0x3000); // these are vendorID and productID I found for my usb device
	if (dev_handle == NULL)
	{
		cout << "Cannot open device" << endl;
		return false;
	}
	else
	{
		std::cout << "Device Opened" << endl;
		libusb_free_device_list(devs, 1); // free the list, unref the devices in it
	}

	// int actual; // used to find out how many bytes were written
	if (libusb_kernel_driver_active(dev_handle, interfaceNumber) == 1)
	{ // find out if kernel driver is attached
		cout << "Kernel Driver Active" << endl;
		if (libusb_detach_kernel_driver(dev_handle, interfaceNumber) == 0) // detach it
			cout << "Kernel Driver Detached!" << endl;
	}

	r = libusb_claim_interface(dev_handle, 1); // claim interface 0 (the first) of device (mine had jsut 1)
	if (r < 0)
	{
		cout << "Cannot Claim Interface" << endl;
		return false;
	}
	std::cout << "Claimed Interface" << std::endl;
	libusbinit = true;
	return true;
}
void uninitlibusb()
{
	if (!libusbinit)
	{
		return;
	}
	// cout << "Data->" << data << "<-" << endl; // just to see the data we want to write : abcd
	// cout << "Writing Data..." << endl;
	// r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN), data, 4, &actual, 0); // my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
	// if(r == 0 && actual == 4)                                                              // we wrote the 4 bytes successfully
	// 	cout << "reading Successful!" << endl;
	// else
	// 	cout << "read Error" << endl;
	using namespace std;
	int r = libusb_release_interface(dev_handle, interfaceNumber); // release the claimed interface
	if (r != 0)
	{
		// cout << "Cannot Release Interface" << endl;
		return;
	}
	// cout << "Released Interface" << endl;

	libusb_close(dev_handle); // close the device we opened
	libusb_exit(ctx);		  // needs to be called to end the
	libusbinit = false;
	return;
}

void getbytes(USBResponse x)
{
	initlibusb();
	int actual;
	int r;
	x.size = 0;
	std::cout << "startGetBytes" << std::endl;
	r = libusb_bulk_transfer(dev_handle, readNX(), (unsigned char *)&x.size, 4, &actual, 10000); // my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
	// std::cout << "actual 1: " << actual << std::endl << std::endl;
	std::cout << "size:" << x.size << std::endl;
	if (x.size = 0)
	{
		uninitlibusb();
		return;
	}
	if (r != 0) // we wrote the 4 bytes successfully// yeah when it was == lol
		std::cout << "read Error1:" << x.size << std::endl;
	x.data[x.size] = {0};
	r = libusb_bulk_transfer(dev_handle, readNX(), (unsigned char *)x.data, x.size, &actual, 10000); // my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
	// std::cout << "actual 2: " << actual << std::endl;
	if (r != 0) // we wrote the 4 bytes successfully// yeah when it was == lol
		std::cout << "read Error2:" << std::endl
				  << "error name thing:" << libusb_error_name(r) << std::endl;
	//std::cout << "after: 0x" << std::to_string(linebuf[0]) << std::to_string(linebuf[2]) << std::to_string(linebuf[3]) << std::to_string(linebuf[4]) << std::endl;
	//r = libusb_bulk_transfer(dev_handle, readNX(), &ee, x.size, (int *)&linebuf[0], 100); // my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
	//if (r != 0)																						// we wrote the 4 bytes successfully// yeah when it was == lol
	//	std::cout << "read Error2:" << linebuf << std::endl;
	//std::cout << "afterest: 0x" << std::to_string(linebuf[0]) << std::to_string(linebuf[2]) << std::to_string(linebuf[3]) << std::to_string(linebuf[4]) << std::endl;
	std::cout << "endGetBytes" << std::endl;
}

//bad
void dobytes(USBResponse &x)
{
	initlibusb();
	int actual;
	int r;
	//unsigned char *ee = (unsigned char *)x.size;
	r = libusb_bulk_transfer(dev_handle, readNX(), (unsigned char *)&x.size, 4, &actual, 0); //my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
	std::cout << x.size << std::endl;														 // my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
	if (r != 0)
		std::cout << "do error!" << std::endl;
	r = libusb_bulk_transfer(dev_handle, readNX(), (unsigned char *)x.data, x.size, (int *)x.data, 0); // my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
	if (r != 0)																						   // we wrote the 4 bytes successfully// yeah when it was == lol
		std::cout << "do Error2" << std::endl;
}

void sendbytes(USBResponse x)
{
	initlibusb();
	int actual;
	int r;

	unsigned char ee = 'e';
	std::cout << "startSendBytes" << std::endl;
	r = libusb_bulk_transfer(dev_handle, sendNX(), (unsigned char *)&x.size, 4, &actual, 0); // my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
	if (r != 0)																				 // we wrote the 4 bytes successfully// yeah when it was == lol
		std::cout << "write Error1:" << libusb_error_name(r) << std::endl;

	r = libusb_bulk_transfer(dev_handle, sendNX(), (unsigned char *)&x.data, x.size, &actual, 0); // my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
	if (r != 0)																					  // we wrote the 4 bytes successfully// yeah when it was == lol
		std::cout << "write Error2" << std::endl;
	std::cout << "endSendBytes" << std::endl;
}

void sendStr(std::string input)
{
	initlibusb();
	int actual;
	int r;

	unsigned char ee = 'e';
	r = libusb_bulk_transfer(dev_handle, sendNX(), (unsigned char *)(input.size() + 2), 4, &actual, 0); // my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
	if (r != 0)																							// we wrote the 4 bytes successfully// yeah when it was == lol
		std::cout << "write Error1:" << libusb_error_name(r) << std::endl;

	r = libusb_bulk_transfer(dev_handle, sendNX(), (unsigned char *)input.c_str(), input.size(), &actual, 0); // my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
	if (r != 0)																								  // we wrote the 4 bytes successfully// yeah when it was == lol
		std::cout << "write Error2" << libusb_error_name(r) << std::endl;
}

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

bool siftThroughDevices(libusb_device *dev)
{
	libusb_device_descriptor desc;
	libusb_get_device_descriptor(dev, &desc);
	if (r < 0)
	{
		std::cout << "failed to get device descriptor" << std::endl;
		return false;
	}
	if (desc.idVendor == 0x057e)
		if (desc.idProduct == 0x3000)
		{
			int cnt = libusb_open(dev, &dev_handle);
			if (cnt < 0)
			{
				std::cout << "Get Device Error" << cnt << std::endl; //there was an error
				return false;
			}
			else
			{
				std::cout << "openeded?" << std::endl;
				return true;
			}
		}
	return false;
}

bool tryOpenNX()
{

	libusb_device **devices;	 //pointer to pointer of device, used to retrieve a list of devices
	libusb_context *ctxx = NULL; //a libusb session
	int rr;						 //for return values
	ssize_t cnt;				 //holding number of devices in list
	rr = libusb_init(&ctxx);	 //initialize a library session
	if (rr < 0)
	{
		std::cout << "Init Error " << rr << std::endl; //there was an error
		return false;
	}
	libusb_set_option(ctxx, LIBUSB_OPTION_USE_USBDK); //set verbosity level to 3, as suggested in the documentation
	cnt = libusb_get_device_list(ctxx, &devices);	  //get the list of devices
	if (cnt < 0)
	{
		std::cout << "Get Device Error" << cnt << std::endl; //there was an error
	}
	std::cout << (int)cnt << " Devices in list." << std::endl; //print total number of usb devices
	ssize_t i;												   //for iterating through the list
	for (i = 1; i < cnt; i++)
	{
		if (siftThroughDevices(devices[i])) //print specs of this device
		{
			libusb_config_descriptor *config;
			libusb_get_config_descriptor(devices[i], 0, &config);

			libusb_free_device_list(devices, 1); // free the list, unref the devices in it
			interfaceNumber = (config->bNumInterfaces - 1);
			if (libusb_kernel_driver_active(dev_handle, interfaceNumber) == 1)
			{ // find out if kernel driver is attached
				std::cout << "Kernel Driver Active " << interfaceNumber << std::endl;
				if (libusb_detach_kernel_driver(dev_handle, interfaceNumber) == 0) // detach it
					std::cout << "Kernel Driver Detached " << interfaceNumber << '!' << std::endl;
			}

			r = libusb_claim_interface(dev_handle, interfaceNumber); // claim interface 0 (the first) of device (mine had jsut 1)
			if (r < 0)
			{
				std::cout << "Cannot Claim Interface " << interfaceNumber << std::endl;
				return false;
			}
			std::cout << "Claimed Interface " << interfaceNumber << std::endl;

			libusbinit = true;
			return true;
		}
	}
	libusb_free_device_list(devices, 1); //free the list, unref the devices in it
	libusb_exit(ctxx);					 //close the session
	return false;
}

int libusbCmdLog()
{
	libusb_device **devices; //pointer to pointer of device, used to retrieve a list of devices
	int rr;					 //for return values
	ssize_t cnt;			 //holding number of devices in list
	rr = libusb_init(&ctx);	 //initialize a library session
	if (rr < 0)
	{
		std::cout << "Init Error " << rr << std::endl; //there was an error
		return 1;
	}
	libusb_set_option(ctx, LIBUSB_OPTION_USE_USBDK); //set verbosity level to 3, as suggested in the documentation
	cnt = libusb_get_device_list(ctx, &devices);	 //get the list of devices
	if (cnt < 0)
	{
		std::cout << "Get Device Error" << std::endl; //there was an error
	}
	std::cout << cnt << " Devices in list." << std::endl; //print total number of usb devices
	ssize_t i;											  //for iterating through the list
	for (i = 1; i < cnt; i++)
	{
		printdev(devices[i]); //print specs of this device
	}
	libusb_free_device_list(devices, 1); //free the list, unref the devices in it
	libusb_exit(ctx);					 //close the session
	return 1;
}

uint64_t getMain()
{
	std::cout << __FUNCTION__ << std::endl;
	if (initlibusb())
		return 0;
	using namespace std;
	int BytesWritten = 0;
	int BytesRead = 0;					   //used to find out how many bytes were written/read
	char command[] = "getMainNsoBase\r\n"; //data to write again
	uint64_t DataIn = 0;				   //data to read
	uint32_t sizeofDataIn = 0;
	uint32_t commandsize; //data to write
	commandsize = sizeof(command);
	libusb_bulk_transfer(dev_handle, (sendNX()), (unsigned char *)&commandsize, sizeof(uint32_t), &BytesWritten, 0); // write data

	libusb_bulk_transfer(dev_handle, (sendNX()), (unsigned char *)command, sizeof(command), &BytesWritten, 0);

	libusb_bulk_transfer(dev_handle, (readNX()), (unsigned char *)&sizeofDataIn, sizeof(uint32_t *), &BytesRead, 0);

	libusb_bulk_transfer(dev_handle, (readNX()), (unsigned char *)&DataIn, sizeof(uint64_t *), &BytesRead, 0); // write to device

	return DataIn;
}

void stopsysmod()
{
	std::cout << __FUNCTION__ << std::endl;
	int BytesWritten = 0;				  //used to find out how many bytes were written/read
	char command[] = "TurnOffSysmod\r\n"; //data to write again
	uint32_t commandsize;				  //data to write
	commandsize = sizeof(command);
	libusb_bulk_transfer(dev_handle, (sendNX()), (unsigned char *)&commandsize, sizeof(uint32_t), &BytesWritten, 0); // write data
	std::cout << "recieved: " << BytesWritten << std::endl;
	libusb_bulk_transfer(dev_handle, (sendNX()), (unsigned char *)command, sizeof(command), &BytesWritten, 0);
	std::cout << "recieved: " << BytesWritten << std::endl;
}

void enablePointerMode()
{
	std::cout << __FUNCTION__ << std::endl;
	int BytesWritten = 0;					  //used to find out how many bytes were written/read
	char command[] = "enablePointerMode\r\n"; //data to write again
	uint32_t commandsize;					  //data to write
	commandsize = sizeof(command);
	libusb_bulk_transfer(dev_handle, (sendNX()), (unsigned char *)&commandsize, sizeof(uint32_t), &BytesWritten, 0); // write data
	std::cout << "recieved: " << BytesWritten << std::endl;
	libusb_bulk_transfer(dev_handle, (sendNX()), (unsigned char *)command, sizeof(command), &BytesWritten, 0);
	std::cout << "recieved: " << BytesWritten << std::endl;
}

void populate_pointers(u32 offsets[], u32 numberOfOffsets)
{
	std::cout << __FUNCTION__ << std::endl;
	int BytesWritten = 0;																								 //used to find out how many bytes were written/read
	libusb_bulk_transfer(dev_handle, (sendNX()), (unsigned char *)&numberOfOffsets, sizeof(uint32_t), &BytesWritten, 0); // write data
	std::cout << "recieved: " << BytesWritten << std::endl;
	for (u32 i = 0; i < numberOfOffsets; i++)
	{
		libusb_bulk_transfer(dev_handle, (sendNX()), (unsigned char *)&offsets[i], sizeof(uint32_t), &BytesWritten, 0); // write data
		std::cout << "recieved: " << BytesWritten << std::endl;
	}
}

void recievePointerData(u8 data[])
{
	std::cout << __FUNCTION__ << std::endl;

	int BytesWritten = 0;																							 //used to find out how many bytes were written/read
	uint32_t commandsize;																							 //data to write
	libusb_bulk_transfer(dev_handle, (readNX()), (unsigned char *)&commandsize, sizeof(uint32_t), &BytesWritten, 0); // write data
	std::cout << "recieved: " << BytesWritten << std::endl;
	libusb_bulk_transfer(dev_handle, (readNX()), (unsigned char *)data, commandsize, &BytesWritten, 0);
	std::cout << "recieved: " << BytesWritten << std::endl;
}
u64 getBaseAddrNX()
{
	std::cout << __FUNCTION__ << std::endl;

	int BytesWritten = 0; //used to find out how many bytes were written/read
	uint32_t commandsize; //data to write
	u64 baseaddr = 0;
	libusb_bulk_transfer(dev_handle, (readNX()), (unsigned char *)&commandsize, sizeof(uint32_t), &BytesWritten, 0); // write data
	std::cout << "recieved: " << BytesWritten << std::endl;
	libusb_bulk_transfer(dev_handle, (readNX()), (unsigned char *)&baseaddr, commandsize, &BytesWritten, 0);
	std::cout << "recieved: " << BytesWritten << std::endl;
	return baseaddr;
}

bool NXDataIsValid(u8 data[0x800])
{
	std::cout << __FUNCTION__ << std::endl;
	const char INVALID[] = "Invalid";
	u32 i;
	for (i = 0; i < sizeof(INVALID); i++)
	{
		if (INVALID[i] == data[i])
			continue;
		else
			return true;
	}
	for (; i < 0x800; i++)
		data[i] = 0x0;
	return false;
}

void BuildID(char DataIn[32])
{
	std::cout << __FUNCTION__ << std::endl;
	if (initlibusb())
		return;
	using namespace std;
	int BytesWritten = 0;
	int BytesRead = 0;				//used to find out how many bytes were written/read
	char command[] = "TitleID\r\n"; //data to write again
	uint32_t sizeIntake = 0;
	uint32_t commandsize; //data to write
	commandsize = sizeof(command);

	libusb_bulk_transfer(dev_handle, (sendNX()), (unsigned char *)&commandsize, sizeof(uint32_t), &BytesWritten, 0); // write data
	libusb_bulk_transfer(dev_handle, (sendNX()), (unsigned char *)command, sizeof(command), &BytesWritten, 0);

	libusb_bulk_transfer(dev_handle, (readNX()), (unsigned char *)&sizeIntake, sizeof(uint32_t *), &BytesRead, 0);
	libusb_bulk_transfer(dev_handle, (readNX()), (unsigned char *)DataIn, sizeIntake, &BytesRead, 0); // write to device

	return;
}

void resetNX()
{
	if (initlibusb())
		return;
	using namespace std;
	cout << __FUNCTION__ << endl;
	int BytesWritten = 0;
	u32 command = UINT32_MAX; //data to write again

	libusb_bulk_transfer(dev_handle, (sendNX()), (unsigned char *)&command, sizeof(uint32_t), &BytesWritten, 0); // write data

	return;
}