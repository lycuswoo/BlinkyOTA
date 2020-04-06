#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <applibs/log.h>
#include <applibs/gpio.h>

#include "epoll_timerfd_utilities.h"

static int buttonPollTimerFd = -1;
static int ledPollTimerFd = -1;
static int buttonAGpioFd = -1;
static int ledGpioFd = -1;
// Termination state
volatile sig_atomic_t terminationRequired = false;

// Button state variables, initilize them to button not-pressed (High)
static GPIO_Value_Type buttonAState = GPIO_Value_High;
static struct timespec sleepTime = { 1, 0 };
static int ledstates = 1;
static int buttonpressed = 0;

int epollFd = -1;



static void LedTimerEventHandler(EventData* eventData)
{


	if (ConsumeTimerFdEvent(ledPollTimerFd) != 0) {
		terminationRequired = true;
		return;
	}

	// Check for LED
	GPIO_Value_Type newLedState;
	int result = GPIO_GetValue(ledGpioFd, &newLedState);
	if (result != 0) {
		Log_Debug("ERROR: Could not read button GPIO: %s (%d).\n", strerror(errno), errno);
		terminationRequired = true;
		return;
	}
	if (newLedState == GPIO_Value_High)
		GPIO_SetValue(ledGpioFd, GPIO_Value_Low);
	else
		GPIO_SetValue(ledGpioFd, GPIO_Value_High);


}
static EventData ledEventData = { .eventHandler = &LedTimerEventHandler };
static void ButtonTimerEventHandler(EventData* eventData)
{


	if (ConsumeTimerFdEvent(buttonPollTimerFd) != 0) {
		terminationRequired = true;
		return;
	}

	// Check for button A press
	GPIO_Value_Type newButtonAState;
	int result = GPIO_GetValue(buttonAGpioFd, &newButtonAState);
	if (result != 0) {
		Log_Debug("ERROR: Could not read button GPIO: %s (%d).\n", strerror(errno), errno);
		terminationRequired = true;
		return;
	}

	// If the A button has just been pressed, send a telemetry message
	// The button has GPIO_Value_Low when pressed and GPIO_Value_High when released
	if (newButtonAState != buttonAState) {
		if (newButtonAState == GPIO_Value_Low) {
			Log_Debug("Button A pressed!\n");
			buttonpressed = 1;
			ledstates = ledstates + 1;
			if (ledstates > 3)
				ledstates = 1;
		}
		// Update the static variable to use next time we enter this routine
		buttonAState = newButtonAState;
	}

	if (buttonpressed)
	{

		if (ledstates == 1)
		{

			sleepTime.tv_sec = 1;
			sleepTime.tv_nsec = 0;
		}
		else if (ledstates == 2)
		{

			sleepTime.tv_sec = 0;
			sleepTime.tv_nsec = 400000000L;
		}
		else if (ledstates == 3)
		{
			sleepTime.tv_sec = 0;
			sleepTime.tv_nsec = 90000000L;
		}
		else
		{
			sleepTime.tv_sec = 1;
			sleepTime.tv_nsec = 0;
			ledstates = 1;
		}

		CloseFdAndPrintError(ledPollTimerFd, "ledPoll");
		ledPollTimerFd =
			CreateTimerFdAndAddToEpoll(epollFd, &sleepTime, &ledEventData, EPOLLIN);
		buttonpressed = 0;
	}
}
// event handler data structures. Only the event handler field needs to be populated.
static EventData buttonEventData = { .eventHandler = &ButtonTimerEventHandler };


int main(void)
{
	// This minimal Azure Sphere app repeatedly toggles GPIO 9, which is the green channel of RGB
	// LED 1 on the MT3620 RDB.
	// Use this app to test that device and SDK installation succeeded that you can build,
	// deploy, and debug an app with Visual Studio, and that you can deploy an app over the air,
	// per the instructions here: https://docs.microsoft.com/azure-sphere/quickstarts/qs-overview
	//
	// It is NOT recommended to use this as a starting point for developing apps; instead use
	// the extensible samples here: https://github.com/Azure/azure-sphere-samples
	Log_Debug(
		"\nVisit https://github.com/Azure/azure-sphere-samples for extensible samples to use as a "
		"starting point for full applications.\n");

	ledGpioFd = GPIO_OpenAsOutput(9, GPIO_OutputMode_PushPull, GPIO_Value_High);
	if (ledGpioFd < 0) {
		Log_Debug(
			"Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n",
			strerror(errno), errno);
		return -1;
	}

	// Open button A GPIO as input
	Log_Debug("Opening Starter Kit Button A as input.\n");
	buttonAGpioFd = GPIO_OpenAsInput(12);
	if (buttonAGpioFd < 0) {
		Log_Debug("ERROR: Could not open button A GPIO: %s (%d).\n", strerror(errno), errno);
		return -1;
	}
	epollFd = CreateEpollFd();
	if (epollFd < 0) {
		return -1;
	}

	// Set up a timer to poll the buttons
	struct timespec buttonPressCheckPeriod = { 0, 1000000 };


	buttonPollTimerFd =
		CreateTimerFdAndAddToEpoll(epollFd, &buttonPressCheckPeriod, &buttonEventData, EPOLLIN);

	ledPollTimerFd =
		CreateTimerFdAndAddToEpoll(epollFd, &sleepTime, &ledEventData, EPOLLIN);
	if (buttonPollTimerFd < 0) {
		return -1;
	}


	while (!terminationRequired) {
		if (WaitForEventAndCallHandler(epollFd) != 0) {
			terminationRequired = true;
		}

	}
	CloseFdAndPrintError(epollFd, "Epoll");
	CloseFdAndPrintError(buttonPollTimerFd, "buttonPoll");
	CloseFdAndPrintError(buttonAGpioFd, "buttonA");
}