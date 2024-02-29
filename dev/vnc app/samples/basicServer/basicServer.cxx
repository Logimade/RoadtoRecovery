/*
Copyright (C) 2016-2017 RealVNC Limited. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#include <vnc/Vnc.h>

/*
 * basicServer sample
 *
 * This cross-platform sample shows how to implement a basic VNC server using
 * the VNC SDK.
 *
 * Two types of connectivity are supported: Cloud-based and direct connectivity (UDP or TCP),
 * with the server permitted to use both mechanisms concurrently.
 *
 * Note: To use direct connectivity you will need to apply an add-on code; a trial
 * code is available from your RealVNC account. You can ignore direct connectivity related
 * code below if you do not intend to use the direct connectivity add-on.
 *
 * The server listens for incoming connections using connectivity details that
 * can be either specified on the command line, or built-in using macro
 * definitions provided below.
 *
 * Each time it starts, the server generates a new random 4-digit password and
 * prints this to the console. A viewer must specify this password when prompted
 * in order to successfully connect.
 */


/* For Cloud connections, either hard-code the Cloud address for the Server OR
 * specify it at the command line. Example Cloud address:
 * LxygGgSrhXQFiLj5M4M.LxyPXzA9sGLkB6pCtJv.devEX1Sg2Txs1CgVuW4.LxyPRsVnXoDoue4Xqm
 */
#define LOCAL_CLOUD_ADDRESS "RH3DABG7XD6RQfSbnr3.RH3PXDPhhfhoRruGt8R.devEX1Sg2Txs1CgVuW4.RH3PRSWGHSV3upVnSYF"

/* Either hard-code the Cloud password associated with this Cloud address OR
 * specify it at the command line. Example Cloud password: KMDgGgELSvAdvscgGfk2
 */
#define LOCAL_CLOUD_PASSWORD "QXdWdm50SAcNJoNwB.ai"

/* To enable direct connectivity you need to copy the content of your
   add-on code into the string below. */
static const char* directConnectivityAddOnCode = "";/*"hhP4WfC4Zu/nlQ3xGFaVtcIzy2y9MWHGcgO61ghIWuKix3U/0W9v6hQ6qnxWzXItqMlbcveThgKsunYKbLEXVom8kVvjyZ4njSBEB0GOGx5wiQxy9wJbiFzlKy8js466F7SQJSbRDjFb5yEPf0My+TofKldNooqiJ3ejC+WYP/9F4HRjuAgnyUZReeu1y1B4I98fLZzC+w0Cdh6JitnS3b3VcUjY34riwzROBvElTLEoY2sgVbUTx2602eOZN7m/RWFm01EQ+pWAvbrNP0stqdH6d9ZviM2QoYjox5GLCGJHb6ubaWgxzyadWjylsFxIAvL3Q+NcDSAj/v9QQYYkvzxkXXFzF7xcBuzXVBPOHVCs6vkivEc8b29sYWPRusa388Iuyxg8AfeaEg4jTJ/ZV7vRQAX0jkmxFze9wXaXbm0HStmgAf2gfOuo1Yjhd26mdAWIzxnjl+VwbOFmdVY1EiilMvgvcpT9S6pVT72rHPCnCAYOG8f8p3nABKRTplnNzIZRnRLo4EHnSlaUCGEd+1Fhh6Gj50Y7ldR6wSrq1skW88Wzuzfe5cAxo0ZHkU9knI6AgEpwM8dH8k21PsnjAR5TXCHAhzzIdMphhG3WVAgrDdNaPG9PrOU9fCZ99t+mMkatZqohR17y+/rF3BD4kUmdwmsIhKin1nc7W241Dqc=";*/

/* For direct connections (UDP or TCP), you must provide an IP listening port number.
   Either edit the macro below OR provide the port number on the command line.
   The default direct IP port number can be specified below by using:
   #define IP_PORT VNC_DIRECT_UDP_DEFAULT_PORT
   Ignore this macro if you are not using the direct connectivity add-on */
#define IP_PORT 0

/* Socket Port for VR device communication */
#define PORT 12345  

/* The following flags indicate the type of connection(s) being used and they
   are set automatically according to user-supplied command line arguments and
   the macro definitions above. Each type of connection is optional.
   If you set any flag to true below then that makes that type of connection
   mandatory i.e. connectivity details MUST be provided via the command line or
   using the macros above. */
bool usingCloud = false;
bool usingDirectConnectivity = false;

/* Number of random digits in the auto-generated server password: */
const int serverPasswordLength = 0;

/* Stores the auto-generated server password: */
char serverPassword[serverPasswordLength + 1];

/* Function prototypes */
bool parseCommandLine(int argc, const char** argv,
                      const char** cloudAddress, const char** cloudPassword,
                      int* port);
bool initializeSDKandAddOns();
vnc_Server* createAndInitServer();
void setupSecurity(vnc_Server* server);
vnc_CloudListener* createCloudListener(vnc_Server* server,
                                       const char* cloudAddress,
                                       const char* cloudPassword);
vnc_DirectUdpListener* createDirectUdpListener(vnc_Server* server, int port);
void waitForEnter();
void usageAdvice();
void showSDKError(const char* errorString);

/* This flag is used to indicate that we need to retain the console after an
   error condition so the user has a chance to see the relevant logging. */
bool needWait = false;

HWND g_hwndButton; // Handle to the button
static HWND hwndEdit, hwndEdit2;
SOCKET s;
WSADATA wsa;
struct sockaddr_in socket_Server;
char* message;
unsigned long addr = inet_addr("192.168.1.216");
unsigned short portShort = htons(12345);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {

        // Create a button when the window is created
        HINSTANCE hInstance = GetModuleHandle(NULL);
        g_hwndButton = CreateWindow(
            "BUTTON",           // predefined class
            "Procedimento 1",         // button text
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles
            10,                 // x position
            10,                 // y position
            110,                // button width (def: 100)
            30,                 // button height (def: 30)
            hwnd,               // parent window
            (HMENU)1,           // button ID
            hInstance,          // instance handle
            NULL                // no additional data
        );

        g_hwndButton = CreateWindow(
            "BUTTON",           // predefined class
            "Procedimento 2",         // button text
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles
            10,                 // x position
            50,                 // y position
            110,                // button width (def: 100)
            30,                 // button height (def: 30)
            hwnd,               // parent window
            (HMENU)2,           // button ID
            hInstance,          // instance handle
            NULL                // no additional data
        );

        g_hwndButton = CreateWindow(
            "BUTTON",           // predefined class
            "Procedimento 3",         // button text
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles
            10,                 // x position
            90,                 // y position
            110,                // button width (def: 100)
            30,                 // button height (def: 30)
            hwnd,               // parent window
            (HMENU)3,           // button ID
            hInstance,          // instance handle
            NULL                // no additional data
        );

        hwndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "192.168.1.216", WS_BORDER | WS_CHILD | WS_VISIBLE | ES_LEFT, 10, 150, 120, 18, hwnd, (HMENU) 4, hInstance, NULL);

        g_hwndButton = CreateWindow(
            "BUTTON",           // predefined class
            "Set IP",         // button text
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles
            130,                 // x position
            150,                 // y position
            50,                 // button width (def: 100)
            20,                 // button height (def: 30)
            hwnd,               // parent window
            (HMENU)5,           // button ID
            hInstance,          // instance handle
            NULL                // no additional data
        );

        hwndEdit2 = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "12345", WS_BORDER | WS_CHILD | WS_VISIBLE | ES_LEFT, 10, 170, 60, 18, hwnd, (HMENU)6, hInstance, NULL);

        g_hwndButton = CreateWindow(
            "BUTTON",           // predefined class
            "Set Port",         // button text
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles
            70,                 // x position
            170,                // y position
            60,                 // button width (def: 100)
            20,                 // button height (def: 30)
            hwnd,               // parent window
            (HMENU)7,           // button ID
            hInstance,          // instance handle
            NULL                // no additional data
        );


        break;
    }
    case WM_COMMAND: {
        // Começa agora código dos casos da escolha dos butões com a parte de enviar mensagens via socket.

        // Handle button click event
        if (LOWORD(wParam) == 1) {
            puts("Button 1 clicked!");
            // Button with ID 1 was clicked
            // Handle button click here

            //printf("Inicio da Criacao do socket\n");      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Socket START @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            printf("\nInitialising Winsock...");
            if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            {
                printf("Failed. Error Code : %d", WSAGetLastError());
                return 1;
            }
            printf("Initialised.");

            if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
            {
                printf("Could not create socket : %d", WSAGetLastError());
            }
            printf("Socket created.\n");

            socket_Server.sin_addr.s_addr = addr;
            socket_Server.sin_family = AF_INET;
            socket_Server.sin_port = portShort;

            //Connect to remote server
            if (connect(s, (struct sockaddr*)&socket_Server, sizeof(socket_Server)) < 0)
            {
                puts("connect error");
                return 1;
            }
            puts("Connected");

            // printf("Final da Criacao do Socket.\n");     // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Socket END @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
           
            // 
            //Send some data
            message = "procedure1";
            if (send(s, message, strlen(message), 0) < 0)
            {
                puts("Send failed");
                return 1;
            }
            puts("Data Sent.");
        }

        if (LOWORD(wParam) == 2) {
            puts("Button 2 clicked!");
            // Button with ID 2 was clicked
            // Handle button click here
            
            //printf("Inicio da Criacao do socket\n");      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Socket START @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            printf("\nInitialising Winsock...");
            if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            {
                printf("Failed. Error Code : %d", WSAGetLastError());
                return 1;
            }
            printf("Initialised.");

            if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
            {
                printf("Could not create socket : %d", WSAGetLastError());
            }
            printf("Socket created.\n");

            socket_Server.sin_addr.s_addr = addr;
            socket_Server.sin_family = AF_INET;
            socket_Server.sin_port = portShort;

            //Connect to remote server
            if (connect(s, (struct sockaddr*)&socket_Server, sizeof(socket_Server)) < 0)
            {
                puts("connect error");
                return 1;
            }
            puts("Connected");

            // printf("Final da Criacao do Socket.\n");     // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Socket END @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            //Send some data
            message = "procedure2";
            if (send(s, message, strlen(message), 0) < 0)
            {
                puts("Send failed");
                return 1;
            }
            puts("Data Sent");
        }

        if (LOWORD(wParam) == 3) {
            puts("Button 3 clicked!");
            // Button with ID 3 was clicked
            // Handle button click here

            //printf("Inicio da Criacao do socket\n");      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Socket START @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            printf("\nInitialising Winsock...");
            if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            {
                printf("Failed. Error Code : %d", WSAGetLastError());
                return 1;
            }
            printf("Initialised.");

            if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
            {
                printf("Could not create socket : %d", WSAGetLastError());
            }
            printf("Socket created.\n");

            socket_Server.sin_addr.s_addr = addr;
            socket_Server.sin_family = AF_INET;
            socket_Server.sin_port = portShort;


            //Connect to remote server
            if (connect(s, (struct sockaddr*)&socket_Server, sizeof(socket_Server)) < 0)
            {
                puts("connect error");
                return 1;
            }
            puts("Connected");

            // printf("Final da Criacao do Socket.\n");     // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Socket END @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            //Send some data
            message = "procedure3";
            if (send(s, message, strlen(message), 0) < 0)
            {
                puts("Send failed");
                return 1;
            }
            puts("Data Sent.");
        }

        if (LOWORD(wParam) == 5){
            int len = GetWindowTextLengthW(hwndEdit) + 1;
            wchar_t* text = new wchar_t[len];

            GetWindowTextW(hwndEdit, text, len);

            char str[20];
            if (wcstombs(str, text, 20) == (size_t)-1) {
                std::cerr << "Error converting wide char to multibyte" << std::endl;
                return EXIT_FAILURE;
            }

            // Print narrow character string (for debugging)
            //std::cout << "Narrow char string: " << str << std::endl;

            // Convert narrow character string to network byte order
            addr = inet_addr(str);
            if (addr == INADDR_NONE) {
                std::cerr << "Invalid IP address" << std::endl;
                return EXIT_FAILURE;
            }
            puts("IP changed!");
        }

        if (LOWORD(wParam) == 7) {
            // Example wide character string representing the port number
            int len = GetWindowTextLengthW(hwndEdit) + 1;
            wchar_t* wport = new wchar_t[len];

            GetWindowTextW(hwndEdit2, wport, len);

            // Convert wide character string to integer
            wchar_t* endPtr;
            unsigned long port = wcstoul(wport, &endPtr, 10); // Base 10

            // Check for conversion errors
            if (*endPtr != L'\0') {
                std::cerr << "Error converting wide char to integer" << std::endl;
                return EXIT_FAILURE;
            }

            // Convert port number to network byte order (big-endian)
            portShort = htons(static_cast<unsigned short>(port));

            puts("Port changed!");
        }

        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        WSACleanup();
        closesocket(s);
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

/*
 * Server main function
 */
int main(int argc, const char** argv)
{
    
    //printf("Inicio da criacao do butao.\n");    // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Início da Interface gráfica @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MyWindowClass";

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window class registration failed!", "Error", MB_ICONERROR);
        return -1;
    }

    HWND hwnd = CreateWindow(
        "MyWindowClass",    // class name
        "My Window",        // window caption
        WS_OVERLAPPEDWINDOW, // window style
        0,      // initial x position
        0,      // initial y position
        400,                // initial width
        300,                // initial height
        NULL,               // parent window
        NULL,               // window menu
        hInstance,          // instance handle
        NULL                // additional application data
    );

    if (!hwnd) {
        MessageBox(NULL, "Window creation failed!", "Error", MB_ICONERROR);
        return -1;
    }

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);


    //printf("Final da Criacao do Butao.\n");     // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Fim da Interface gráfica @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
   

  int exitCode = 1;

  /* Parameter initialisation */
  const char* cloudAddress = LOCAL_CLOUD_ADDRESS;
  const char* cloudPassword = LOCAL_CLOUD_PASSWORD;

  vnc_Server* server = 0;
  vnc_CloudListener* cloudListener = 0;

  /* These are only relevant if you are using the direct connectivity add-on */
  int port = IP_PORT;
  vnc_DirectUdpListener* directUdpListener = 0;

  /* Parse command line */
  if (!parseCommandLine(argc, argv, &cloudAddress, &cloudPassword, &port)) {
    return exitCode;
  }

  /* Create a logger which outputs to stderr.
     This is created before initializing the server so we receive logging
     as soon as the server starts up. */
  vnc_Logger_createStderrLogger();

  /* Create a file DataStore for storing persistent data for the server.
     Ideally this would be created in a directory that only the server
     user has access to. */
  if (!vnc_DataStore_createFileStore("fileStore.txt")) {
    waitForEnter();
    return exitCode;
  }

  /* Initialize SDK and optional Add-Ons */
  if (!initializeSDKandAddOns()) {
    return exitCode;
  }

  /* Create server and configure callbacks */
  server = createAndInitServer();
  if (server) {
#ifdef _DEBUG
    /* Not recommended for release builds. Enabling "full" logging is useful for seeing what's 
       going on when first trying a system. See Logger.h for more details */
    vnc_Logger_setLevel(vnc_Logger_Full);
#endif

    /* Setup security */
    setupSecurity(server);

    /* Listen on each transport that we intend to use. */
    bool proceed = true;
    if (usingCloud) {
      cloudListener = createCloudListener(server, cloudAddress, cloudPassword);
      if (!cloudListener) proceed = false;
    }
    if (proceed && usingDirectConnectivity) {
      directUdpListener = createDirectUdpListener(server, port);
      if (!directUdpListener) proceed = false;
    }


    /* Only continue if we have successfully created all intended listeners */
    if (proceed) {
      /* Run the EventLoop */
      vnc_EventLoop_run();
      exitCode = 0;
    }
  }
  if (needWait)
    waitForEnter();

  /* Cleanup */
  if (cloudListener) vnc_CloudListener_destroy(cloudListener);
  if (directUdpListener) vnc_DirectUdpListener_destroy(directUdpListener);
  if (server) vnc_Server_destroy(server);
  
  /* Shutdown the SDK */
  vnc_shutdown();
  return exitCode;
}

/*
 * Provide usage information on console.
 */
void usageAdvice()
{
  std::cerr << "Usage:  ./basicServer [cloudAddress cloudPassword] "
               "[directConnectivityPortNumber]" << std::endl;
  waitForEnter();
}

/*
 * Extract port number from command line argument.
 */
int extractPortNum(const char* arg)
{
  char* tailPtr;
  int port = strtol(arg, &tailPtr, 10);
  if (*tailPtr) return 0; /* tailPtr should point to null */
  return port;
}

/*
 * Parse the command line to obtain connectivity details to be used when
 * listening for incoming connections. A simplistic approach is adopted:
 *
 *   3 arguments - Cloud and direct connectivity to be used
 *                 [cloudAddress cloudPassword] [directConnectivityPortNumber]
 *
 *   2 arguments - Cloud connectivity to be used
 *                 [cloudAddress cloudPassword]

 *   1 argument  - direct connectivity to be used
 *                 [directConnectivityPortNumber]

 *   0 arguments - the built-in macros must be set appropriately
 */
bool parseCommandLine(int argc, const char** argv,
                      const char** cloudAddress,
                      const char** cloudPassword,
                      int* ipPort)
{
  bool badArgs = false;

  /* Parse any supplied command line arguments */
  if (argc >= 1 && argc <= 4) {
    if (argc == 4) { /* Cloud and direct connectivity arguments */
      *cloudAddress = argv[1];
      *cloudPassword = argv[2];
      *ipPort = extractPortNum(argv[3]);
      usingCloud = usingDirectConnectivity = true;
    } else if (argc == 3) { /* Cloud arguments only */
      *cloudAddress = argv[1];
      *cloudPassword = argv[2];
      usingCloud = true;
    } else  if (argc == 2) { /* direct connectivity argument only */
      *ipPort = extractPortNum(argv[1]);
      usingDirectConnectivity = true;
    } else { /* Examine initial values set by macros */
      if (**cloudAddress || **cloudPassword) usingCloud = true;
      if (*ipPort) usingDirectConnectivity = true;
    }

    /* Check if all required connectivity details are provided */
    if (usingCloud && (!**cloudAddress || !**cloudPassword)) {
      std::cerr << "You must provide a valid Cloud address and password"
                << std::endl;
      badArgs = true;
    }
    if (usingDirectConnectivity && !*ipPort) {
      std::cerr << "You must provide a valid IP port number"
                << std::endl;
      badArgs = true;
    }
    if (!usingCloud && !usingDirectConnectivity) {
      std::cerr << "No connectivity information provided." << std::endl;
      badArgs = true;
    }
  } else badArgs = true; /* Invalid number of arguments */

  if (badArgs) {
    usageAdvice();
    return false;
  }
  return true;
}

/*
 * Initialize SDK and Add-ons
 */
bool initializeSDKandAddOns()
{
  /* Initialize the SDK */
  if (!vnc_init()) {
    showSDKError("Failed to initialize VNC SDK");
    return false;
  }

  /* Enable direct connectivity Add-On */
  if (usingDirectConnectivity && !vnc_enableAddOn(directConnectivityAddOnCode)) {
    showSDKError("Failed to enable direct connectivity add-on");
    return false;
  }

  return true;
}

/*
 * Connection callback - connection started
 */
void connectionStarted(void* userData,
                       vnc_Server* server,
                       vnc_Connection* connection)
{
  const char* peerAddress = vnc_Server_getPeerAddress(server, connection);
  std::cout << "Viewer " << peerAddress << " connected" << std::endl;
}

/*
 * Connection callback - connection ended
 */
void connectionEnded(void* userData,
                     vnc_Server* server,
                     vnc_Connection* connection)
{
  const char* peerAddress = vnc_Server_getPeerAddress(server, connection);
  std::cout << "Viewer " << peerAddress << " disconnected" << std::endl;
}

/*
 * Create Server, setting up connection callbacks
 */
vnc_Server* createAndInitServer()
{
  /* Initialize the server. Note that the vncagent path is not specified, so it
     should be found in the same directory as the main program */
  vnc_Server* server = vnc_Server_create(0);
  if (!server) {
    showSDKError("Failed to initialize server");
    return 0;
  }

  /* Setup connection callbacks */
  vnc_Server_ConnectionCallback connectionCallback = {
    connectionStarted,
    connectionEnded,
  };
  vnc_Server_setConnectionCallback(server, &connectionCallback, 0);

  return server;
}

/*
 * Security callback - checks if a username is required. Always false.
 */
static vnc_bool_t isUserNameRequired(void* userData, vnc_Server* server,
                                     vnc_Connection* connection)
{
  /* Don't prompt for a username when accessing this server, just a password
     is required */
  return vnc_false;
}

/*
 * Security callback - checks supplied authentication password.
 */
static int authenticateUser(void* userData, vnc_Server* server,
                            vnc_Connection* connection, const char* username,
                            const char* password)
{
  /* Check that the password supplied by the connecting viewer is the same as
     the server's auto-generated random password. If so, allow the connection
     with all permissions, otherwise do not allow the connection. */
  if (strcmp(password, serverPassword) == 0) {
    return vnc_Server_PermAll;
  }
  return 0;
}


/* 
 * Return true if we can print colours using ANSI escape sequences.
 * Enable them first, if required (windows)
 */
bool supportsAnsi()
{
#ifdef _WIN32
  static bool modeset = false;
  static DWORD cmode = 0;

  /* Once per instance, check if we can use ANSI colours in the console */
  if (!modeset) {
    modeset = true;

    HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hStd != INVALID_HANDLE_VALUE && GetConsoleMode(hStd, &cmode)) {
      /* Enable ANSI escape sequences in the windows console with ENABLE_VIRTUAL_TERMINAL_PROCESSING flag (4) */
      if (!SetConsoleMode(hStd, cmode = cmode | 4))
        cmode = 0;
    }
  }

  return cmode != 0;
#else
  return true;
#endif  
}

/* Make this text stand out */
void emphasise(const char *text)
{
  // Print in green: 92m
  if (supportsAnsi())
    std::cout << "\x1b[92m" << text << "\x1b[0m";
  else
    std::cout << text;
}

/*
 * Setup server security-related settings and callbacks
 */
void setupSecurity(vnc_Server* server)
{
  /* Generate the server password using a series of random digits.
     Note that for generating random passwords for production use, it is
     advisable to use a better source of random numbers. The C library rand()
     function is only used here as a simple example as it is available on all
     platforms. */
  srand((unsigned int)time(0));
  for (int i=0; i<serverPasswordLength; ++i)
    serverPassword[i] = '0' + (rand() % 10);
  serverPassword[serverPasswordLength] = '\0';

  std::cout << "Server password is: ";
  emphasise(serverPassword);
  std::cout << std::endl;

  /* Setup security callback */
  vnc_Server_SecurityCallback securityCallback = {
    0, /* verifyPeer not required */
    isUserNameRequired,
    0, /* isPasswordRequired always returns true if not specified */
    authenticateUser
  };
  vnc_Server_setSecurityCallback(server, &securityCallback, 0);
}

/*
 * Cloud listener callback - failed
 */
static void listeningFailed(void* userData, vnc_CloudListener* listener,
                            const char* cloudError, int suggestedRetryTime)
{
  std::cerr << "VNC Cloud listening error: " << cloudError << std::endl;
  needWait = true;
  vnc_EventLoop_stop();
}

/*
 * Cloud listener callback - status changed
 */
static void listeningStatusChanged(void* userData, vnc_CloudListener* listener,
                                   vnc_CloudListener_Status status)
{
  if(status == vnc_CloudListener_StatusSearching) {
    std::cout << "The listener is in the process of establishing an "
      "association with VNC Cloud" << std::endl;
  } else {
    std::cout << "Listening for VNC Cloud connections" << std::endl;
  }
}

/*
 * Start listening for connections made via VNC Cloud
 */
vnc_CloudListener* createCloudListener(vnc_Server* server,
                                       const char* cloudAddress,
                                       const char* cloudPassword)
{
  std::cout << "Signing in to VNC Cloud" << std::endl;
  vnc_CloudListener_Callback listenerCallback = {
    listeningFailed,
    0, /* filterConnection not required here */
    listeningStatusChanged
  };
  vnc_CloudListener* listener =
      vnc_CloudListener_create(cloudAddress, cloudPassword,
                               vnc_Server_getConnectionHandler(server),
                               &listenerCallback, 0);

  if (!listener) {
    showSDKError("Could not create Cloud listener");
  }
  return listener;
}

/*
 * RsaKey callback - key details ready
 */
void rsaKeyDetailsReady(void* userData,
                        const vnc_DataBuffer* rsaPublic,
                        const char* hexFingerprint,
                        const char* catchphraseFingerprint)
{
  std::cout << "Server id is: " << hexFingerprint << std::endl;
  std::cout << "Server catchphrase is: " << catchphraseFingerprint << std::endl;

}

/*
 * Start listening for connections made via UDP direct connectivity
 * Ignore this if you do not intend to use the direct connectivity add-on
 */
vnc_DirectUdpListener* createDirectUdpListener(vnc_Server* server, int port)
{
  std::cout << "Listening for Direct UDP connections" << std::endl;
  /* This listens on ALL addresses and does not employ callbacks */
  vnc_DirectUdpListener* listener =
      vnc_DirectUdpListener_create(port, 0,
                                   vnc_Server_getConnectionHandler(server),
                                   0, 0);

  if (!listener) {
    showSDKError("Could not create Direct UDP listener");
  } else {
    /* If DirectConnectivity is being used, request the RSA key details to display so
       that viewers can verify they are connecting to this server. */
    vnc_RsaKey_Callback keyCallback = {
      rsaKeyDetailsReady
    };
    vnc_RsaKey_getDetails(&keyCallback, 0, true);
  }
  return listener;
}


/*
 * Prompt and wait for user to press enter before continuing.
 * This is used when displaying error messages, allowing the user to read the
 * message on systems where the terminal closes when the program exits.
 */
void waitForEnter()
{
  std::cout << "Press ENTER to continue..." << std::endl;
  std::cin.ignore();
}

/*
 * Provide SDK error information on the console.
 */
void showSDKError(const char* errorString)
{
  std::cerr << errorString << ": " << vnc_getLastError() << std::endl;
  waitForEnter();
}

#ifdef WIN32
#include <Windows.h>
/*
 * Install a safer Ctrl+C/Ctrl+Break/End-Task handler on Windows. These run on
 * a newly injected thread, and the default handler unsafely unloads DLLs
 * while they may still be in-use on the main thread.
 *
 * This is necessary as the SDK's threading model does not allow unloading the
 * SDK from one thread while the event loop is still running on another thread.
 */
static struct Win32ConsoleCleanup {
  Win32ConsoleCleanup() { SetConsoleCtrlHandler(&cleanup, TRUE); }
  static BOOL WINAPI cleanup(DWORD dwCtrlType)
  {
    switch (dwCtrlType) {
    case CTRL_C_EVENT: case CTRL_BREAK_EVENT: case CTRL_CLOSE_EVENT:
      /* Tell the main thread to stop its event loop and exit cleanly,
         which will stop all threads including this one. */
      vnc_EventLoop_stop();
      Sleep(500);
      /* The main thread took too long to exit, so hard-kill the process
         with TerminateProcess, since ExitProcess is only safe on the main
         thread, and the default handler isn't safe to return to. */
      TerminateProcess(GetCurrentProcess(), STATUS_CONTROL_C_EXIT);
      return TRUE;  /* This is unreachable. */
    }
    return FALSE;
  }
} win32Cleanup;
#endif
