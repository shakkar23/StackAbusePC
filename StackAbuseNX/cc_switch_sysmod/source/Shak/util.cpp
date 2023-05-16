#ifndef UTIL_CPP
#define UTIL_CPP
#include "util.hpp"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <unistd.h>

/*int setupServerSocket() {
	int lissock;
	int yes = 1;
	struct sockaddr_in server;
	lissock = socket(AF_INET, SOCK_STREAM, 0);

	setsockopt(lissock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	server.sin_family      = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port        = htons(6000);

	while(bind(lissock, (struct sockaddr*)&server, sizeof(server)) < 0) {
		svcSleepThread(1e+9L);
	}
	listen(lissock, 3);
	return lissock;
}*/

u64 parseStringToInt(char *arg)
{
	if (strlen(arg) > 2)
	{
		if (arg[1] == 'x')
		{
			u64 ret = strtoul(arg, NULL, 16);
			return ret;
		}
	}
	u64 ret = strtoul(arg, NULL, 10);
	return ret;
}

u8 *parseStringToByteBuffer(char *arg, u64 *size)
{
	char toTranslate[2];
	int length = strlen(arg);
	bool isHex = false;

	if (length > 2)
	{
		if (arg[1] == 'x')
		{
			isHex = true;
			length -= 2;
			arg = &arg[2]; // cut off 0x
		}
	}

	bool isFirst = true;
	bool isOdd = (length % 2 == 1);
	u64 bufferSize = length / 2;
	if (isOdd)
		bufferSize++;
	u8 *buffer = (u8 *)malloc(bufferSize);

	u64 i;
	for (i = 0; i < bufferSize; i++)
	{
		if (isOdd)
		{
			if (isFirst)
			{
				toTranslate[0] = '0';
				toTranslate[1] = arg[i];
			}
			else
			{
				toTranslate[0] = arg[(2 * i) - 1];
				toTranslate[1] = arg[(2 * i)];
			}
		}
		else
		{
			toTranslate[0] = arg[i * 2];
			toTranslate[1] = arg[(i * 2) + 1];
		}
		isFirst = false;
		if (isHex)
		{
			buffer[i] = strtoul(toTranslate, NULL, 16);
		}
		else
		{
			buffer[i] = strtoul(toTranslate, NULL, 10);
		}
	}
	*size = bufferSize;
	return buffer;
}

HidNpadButton parseStringToButton(char *arg)
{
	if (strcmp(arg, "A") == 0)
	{
		return HidNpadButton_A;
	}
	else if (strcmp(arg, "B") == 0)
	{
		return HidNpadButton_B;
	}
	else if (strcmp(arg, "X") == 0)
	{
		return HidNpadButton_X;
	}
	else if (strcmp(arg, "Y") == 0)
	{
		return HidNpadButton_Y;
	}
	else if (strcmp(arg, "RSTICK") == 0)
	{
		return HidNpadButton_StickR;
	}
	else if (strcmp(arg, "LSTICK") == 0)
	{
		return HidNpadButton_StickL;
	}
	else if (strcmp(arg, "L") == 0)
	{
		return HidNpadButton_L;
	}
	else if (strcmp(arg, "R") == 0)
	{
		return HidNpadButton_R;
	}
	else if (strcmp(arg, "ZL") == 0)
	{
		return HidNpadButton_ZL;
	}
	else if (strcmp(arg, "ZR") == 0)
	{
		return HidNpadButton_ZR;
	}
	else if (strcmp(arg, "PLUS") == 0)
	{
		return HidNpadButton_Plus;
	}
	else if (strcmp(arg, "MINUS") == 0)
	{
		return HidNpadButton_Minus;
	}
	else if (strcmp(arg, "DLEFT") == 0)
	{
		return HidNpadButton_Left;
	}
	else if (strcmp(arg, "DUP") == 0)
	{
		return HidNpadButton_Up;
	}
	else if (strcmp(arg, "DRIGHT") == 0)
	{
		return HidNpadButton_Right;
	}
	else if (strcmp(arg, "DDOWN") == 0)
	{
		return HidNpadButton_Down;
		// } else if(strcmp(arg, "HOME") == 0) {
		// 	return KEY_HOME;AppletSystemButtonType_HomeButtonLongPressing
		// } else if(strcmp(arg, "CAPTURE") == 0) {
		// 	return KEY_CAPTURE;
	}
	return HidNpadButton_A; // I guess lol
}
#endif