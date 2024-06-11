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

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <sstream>
#include <vnc/Vnc.h>
#include <thread>

#include "BasicViewerWindow.h"

/*
 * basicViewerWin sample
 * 
 * This sample shows how to implement a basic VNC viewer using the VNC SDK
 * running on Windows. 
 *
 * Two types of connectivity are supported: 
 * Cloud-based and direct connectivity (UDP or TCP). 
 * A viewer can only use one of these mechanisms at a time.
 *
 * Note: To use direct connectivity (UDP or TCP) you will need to apply an add-on code; a trial
 * code is available from your RealVNC account. You can ignore direct connectivity related
 * code below if you do not intend to use the direct connectivity add-on.
 *
 * The viewer attempts to connect to a server, using either Cloud-based or
 * direct connectivity according to user-supplied connectivity details.
 * These details can be provided on the command line or built-in using the macro
 * definitions below.
 */

/* For Cloud connections, either hard-code the Cloud address for the Viewer OR
 * specify it at the command line. Example Cloud address:
 * LxygGgSrhXQFiLj5M4M.LxyPXzA9sGLkB6pCtJv.devEX1Sg2Txs1CgVuW4.LxyPRsVnXoDoue4Xqm
 */
#define LOCAL_CLOUD_ADDRESS "RH3DABG7XD6RQfSbnr3.RH3PXDPhhfhoRruGt8R.devEX1Sg2Txs1CgVuW4.RH3PRnY2yHqGChxVenM"
#define LOCAL_CLOUD_PASSWORD "cc6--f12ir6fo4P36Ec8"
#define PEER_CLOUD_ADDRESS "RH3DABG7XD6RQfSbnr3.RH3PXDPhhfhoRruGt8R.devEX1Sg2Txs1CgVuW4.RH3PRSWGHSV3upVnSYF"

/* To enable direct connectivity you need to copy the content of your
   add-on code into the string below. */
static const char* directConnectivityAddOnCode = ""; /*hhP4WfC4Zu / nlQ3xGFaVtcIzy2y9MWHGcgO61ghIWuKix3U / 0W9v6hQ6qnxWzXItqMlbcveThgKsunYKbLEXVom8kVvjyZ4njSBEB0GOGx5wiQxy9wJbiFzlKy8js466F7SQJSbRDjFb5yEPf0My + TofKldNooqiJ3ejC + WYP / 9F4HRjuAgnyUZReeu1y1B4I98fLZzC + w0Cdh6JitnS3b3VcUjY34riwzROBvElTLEoY2sgVbUTx2602eOZN7m / RWFm01EQ + pWAvbrNP0stqdH6d9ZviM2QoYjox5GLCGJHb6ubaWgxzyadWjylsFxIAvL3Q + NcDSAj / v9QQYYkvzxkXXFzF7xcBuzXVBPOHVCs6vkivEc8b29sYWPRusa388Iuyxg8AfeaEg4jTJ / ZV7vRQAX0jkmxFze9wXaXbm0HStmgAf2gfOuo1Yjhd26mdAWIzxnjl + VwbOFmdVY1EiilMvgvcpT9S6pVT72rHPCnCAYOG8f8p3nABKRTplnNzIZRnRLo4EHnSlaUCGEd + 1Fhh6Gj50Y7ldR6wSrq1skW88Wzuzfe5cAxo0ZHkU9knI6AgEpwM8dH8k21PsnjAR5TXCHAhzzIdMphhG3WVAgrDdNaPG9PrOU9fCZ99t + mMkatZqohR17y + / rF3BD4kUmdwmsIhKin1nc7W241Dqc = ";*/

/* For direct connectivity by IP you must provide the server's IP host address
   and port number. Either edit the macros below OR provide these connection
   details on the command line.
   The default Direct IP port number can be specified below by using:
   #define IP_PORT VNC_DIRECT_UDP_DEFAULT_PORT
   Ignore these macros if you are not using the direct connectivity add-on */
#define HOST_ADDRESS ""
#define IP_PORT 0


   // Function prototypes for login/register functions
LRESULT CALLBACK LoginRegisterWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RegisterWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int RegisterUser(const char* username, const char* password);
int LoginUser(const char* username, const char* password);

#define ID_EDIT_USERNAME 1
#define ID_EDIT_PASSWORD 2
#define ID_BUTTON_REGISTER 3
#define ID_BUTTON_LOGIN 4

typedef struct {
    char username[30];
    char password[30];
} User;

bool validLogin = false;
bool isAdmin = true;

char CURL_IP[18] = "localhost";
char CURL_PORT[6] = "8000";
char CURL_CONTENT_TYPE[30] = "application/json";


/* The value of this flag is set automatically according to the user-supplied
   command line arguments and macro definitions above. Cloud connectivity is
   presumed by default here. */
bool usingCloud = true;

/* Function prototypes */
void showSDKError(const char* errorString);
bool parseCommandLine(int argc, const char **argv,
                      const char** localCloudAddress,
                      const char** localCloudPassword,
                      const char** peerCloudAddress,
                      int* ipPort, const char** ipHostAddress);
bool initializeSDKandAddOns();

bool makeCloudConnection(const char* localCloudAddress,
                         const char* localCloudPassword,
                         const char* peerCloudAddress,
                         vnc_Viewer* viewer);
bool makeDirectUdpConnection(const char* hostAddress, int port,
                             vnc_Viewer* viewer);
void runEventLoop(vnc_Viewer* viewer);



// Callback function for requesting user credentials
void requestUserCredentials(void* userData, vnc_Viewer* viewer, vnc_bool_t needUser, vnc_bool_t needPasswd) {
    // Cast userData to the appropriate type if needed
    // This data can be passed to the callback when setting it up

    // Example prompt for username and/or password
    std::string username, password;
    if (needUser) {
        //std::cout << "Enter username: ";
        username = "";
        //std::cin >> username;
        // Example: Send username to server
    }
    if (needPasswd) {
        //std::cout << "Enter password: ";
        password = "";
        //std::cin >> password;
        // Example: Send password to server
    }

    // Example: Send the authentication response to the server
    vnc_status_t status = vnc_Viewer_sendAuthenticationResponse(viewer, vnc_true, username.c_str(), password.c_str());
    if (status != vnc_success) {
        std::cerr << "Failed to send authentication response: " << vnc_getLastError() << std::endl;
        // Handle error
    }
}


// Secondary window thread function
DWORD WINAPI SecondaryWindowThread(LPVOID lpParameter) {
    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = RegisterWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MyWindowClass";

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window class registration failed!", "Error", MB_ICONERROR);
        return -1;
    }

    HWND hwnd = CreateWindow(
        "MyWindowClass",    // class name
        "ADMIN: Users Registration",        // window caption
        WS_OVERLAPPEDWINDOW, // window style
        CW_USEDEFAULT,      // initial x position
        CW_USEDEFAULT,      // initial y position
        300,                // initial width
        200,                // initial height
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

    // Message loop for the secondary window
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

/*
 * main function - validates cloud addresses, initializes the SDK and the
 * viewer, creates connection handler
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
  int exitCode = 1;

  WNDCLASSEX wc;
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = 0;
  wc.lpfnWndProc = LoginRegisterWindowProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = NULL;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = "LoginRegisterWindowClass";
  wc.hIconSm = NULL;

  if (!RegisterClassEx(&wc))
  {
      // Handle registration failure
      MessageBox(NULL, "Window class registration failed!", "Error", MB_ICONERROR);
      return 1;
  }

  // Create the login/register window
  HWND hwnd = CreateWindowEx(0, "LoginRegisterWindowClass", "Login", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 300, 200, NULL, NULL, hInstance, NULL);

  if (!hwnd)
  {
      DWORD dwError = GetLastError();
      char errorMessage[256];
      sprintf(errorMessage, "Window creation failed with error code: %lu", dwError);
      MessageBox(NULL, errorMessage, "Error", MB_ICONERROR);
      return 1;
  }

  // Display the window
  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);

  // Message loop
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0))
  {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
  }

  //return msg.wParam;
  
  if (!validLogin) 
  {
      return 1;
  }

  
  if (isAdmin) {
      // Create a new thread for the secondary window
      std::thread secondaryWindowThread(SecondaryWindowThread, nullptr);
      secondaryWindowThread.detach();
  }
 


  /* Parameter initialisation */
  const char* localCloudAddress = LOCAL_CLOUD_ADDRESS;
  const char* localCloudPassword = LOCAL_CLOUD_PASSWORD;
  const char* peerCloudAddress = PEER_CLOUD_ADDRESS;


  vnc_Viewer* viewer = 0;
  BasicViewerWindow* win = 0;

  /* These are only relevant if you are using the direct connectivity add-on */
  const char* hostAddress = HOST_ADDRESS;
  int port = IP_PORT;
  
  /* Parse command line */
  if (!parseCommandLine(__argc, (const char**)__argv,
                        &localCloudAddress, &localCloudPassword,
                        &peerCloudAddress,
                        &port, &hostAddress)) {
    return exitCode;
  }

  /* Create a logger which outputs to a file in the current working directory */
  vnc_Logger_createFileLogger("logfile.txt");

  /* Create a registry DataStore for storing peristent data for the viewer
     under the specified registry key. */
  if (!vnc_DataStore_createRegistryStore(
    "HKEY_CURRENT_USER\\Software\\TestCompany\\basicViewerWin")) {
    std::cerr << "Failed to create Registry data store: " << vnc_getLastError()
              << std::endl;
    return exitCode;
  }

  /* Initialize SDK and optional Add-Ons */
  if (initializeSDKandAddOns()) {
 
    /* Create the viewer and window */
    if (!(viewer = vnc_Viewer_create())) {
      showSDKError("Failed to create viewer");
    }
    else {
      win = new BasicViewerWindow(viewer);

      // Handle the authentication process
      vnc_Viewer_AuthenticationCallback authCallback;
      authCallback.requestUserCredentials = &requestUserCredentials;
      vnc_Viewer_setAuthenticationCallback(viewer, &authCallback, NULL);


      /* Make a connection to the server */
       if (usingCloud) {
        if (makeCloudConnection(localCloudAddress, localCloudPassword,
          peerCloudAddress, viewer)) {
          exitCode = 0;

        }
      }
      else {
        if (makeDirectUdpConnection(hostAddress, port, viewer)) {
          exitCode = 0;
        }
      }
      if (exitCode == 0) {
        /* Connection object has been constructed. */
        runEventLoop(viewer);
      }
      delete win;
    }
  }

  /* Shutdown the SDK */
  vnc_shutdown();
  return exitCode;
}

/*
* Run the Windows message loop while the connection is running
*/

void runEventLoop(vnc_Viewer* viewer)
{
  MSG msg;
  int nextExpiry = 0;
  HANDLE events[MAXIMUM_WAIT_OBJECTS];

  while (vnc_Viewer_getConnectionStatus(viewer) != vnc_Viewer_Disconnected) {
    /* Check whether there are messages or events to be processed immediately */
    bool haveMsg = PeekMessage(&msg, (HWND)0, 0, 0, PM_REMOVE) != 0;
    int nEvents = vnc_EventLoopWin_getEvents(events);
    DWORD waitResult = WaitForMultipleObjects(nEvents, events, FALSE, 0);

    /* If there are no messages or events then block until there are */
    if (!haveMsg && waitResult == WAIT_TIMEOUT) {
      waitResult = MsgWaitForMultipleObjects(
        nEvents, events, FALSE, nextExpiry, QS_ALLINPUT);
    }

    /* Process event */
    HANDLE h = NULL;
    if (waitResult >= WAIT_OBJECT_0 &&
        (int)waitResult < WAIT_OBJECT_0 + nEvents)
      h = events[waitResult - WAIT_OBJECT_0];

    nextExpiry = vnc_EventLoopWin_handleEvent(h);


    /* Process messages */
    if (haveMsg) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
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
*   3 arguments - Cloud connectivity to be used
*                 [localCloudAddress localCloudPassword peerCloudAddress]
*
*   2 arguments - direct connectivity UDP/TCP to be used
*                 [DirectIpHostAddress DirectIpPortNumber]
*
*   0 arguments - the built-in macros must be set appropriately
*/
bool parseCommandLine(int argc, const char** argv,
  const char** localCloudAddress,
  const char** localCloudPassword,
  const char** peerCloudAddress,
  int* ipPort, const char** ipHostAddress)
{
  bool badArgs = false;

  /* Parse any supplied command line arguments */
  if (argc == 4 || argc == 3 || argc == 1) {
    if (argc == 4) {  /* Cloud arguments */
      *localCloudAddress = argv[1];
      *localCloudPassword = argv[2];
      *peerCloudAddress = argv[3];
    }
    else  if (argc == 3) {  /* Direct UDP arguments */
      *ipHostAddress = argv[1];
      *ipPort = extractPortNum(argv[2]);
      usingCloud = false;
    }
    else { /* Examine the initial values set by macros */
      if (**localCloudAddress || **localCloudPassword || **peerCloudAddress)
        usingCloud = true;
      else if (*ipPort || **ipHostAddress)
        usingCloud = false;
    }

    /* Check if all required connectivity details are provided */
    if (usingCloud && (!**localCloudAddress ||
      !**localCloudPassword ||
      !**peerCloudAddress)) {
      badArgs = true;
    }
    else if (!usingCloud && (!*ipPort || !**ipHostAddress)) {
      badArgs = true;
    }
  }
  else badArgs = true; /* Invalid number of arguments */

  if (badArgs) {
    MessageBox(0, "You must specify a local Cloud Address, password and \n"
      "peer Cloud Address, or direct connectivity host address and port number", "Error", MB_OK);

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

  /* Enable Add-ons */

  /* For each add-on you intend to enable you must apply an add-on
     code, as shown for direct connectivity in the example below. */

  /* direct connectivity add-on */
  if (!usingCloud && !vnc_enableAddOn(directConnectivityAddOnCode)) {
    showSDKError("Failed to enable direct connectivity add-on");
    return false;
  }

  return true;
}

/*
* Make a Cloud connection
*/
bool makeCloudConnection(const char* localCloudAddress,
  const char* localCloudPassword,
  const char* peerCloudAddress,
  vnc_Viewer* viewer)
{
  vnc_CloudConnector* cloudConnector = 0;
  /* Create connector */
  if (!(cloudConnector = vnc_CloudConnector_create(localCloudAddress,
    localCloudPassword))) {
    showSDKError("Failed to create Cloud connector");
    return false;
  }
  /* Connect*/
  std::cout << "Connecting via VNC Cloud, local address: " << localCloudAddress
    << " peer address: " << peerCloudAddress << std::endl;
  if (!vnc_CloudConnector_connect(cloudConnector, peerCloudAddress,
    vnc_Viewer_getConnectionHandler(viewer))) {
    showSDKError("Failed to make VNC Cloud connection");
    vnc_CloudConnector_destroy(cloudConnector);
    return false;
  }
  vnc_CloudConnector_destroy(cloudConnector); /* Connector no longer required */
  return true;
}

/*
* Make a direct connectivity UDP connection.
* Ignore this if you do not intend to use the direct connectivity add-on
*/
bool makeDirectUdpConnection(const char* hostAddress,
  int port,
  vnc_Viewer* viewer)
{
  vnc_DirectUdpConnector* udpConnector = 0;
  /* Create Direct UDP connector */
  if (!(udpConnector = vnc_DirectUdpConnector_create())) {
    showSDKError("Failed to create Direct UDP connector");
    return false;
  }
  /* Connect */
  std::cout << "Connecting to host address: " << hostAddress
    << " port: " << port << std::endl;

  if (!vnc_DirectUdpConnector_connect(udpConnector, hostAddress, port, vnc_Viewer_getConnectionHandler(viewer))) {
    showSDKError("Failed to start connection");
    vnc_DirectUdpConnector_destroy(udpConnector);
    return false;
  }
  vnc_DirectUdpConnector_destroy(udpConnector);
  return true;
}

/*
* Provide SDK error information on the console.
*/
void showSDKError(const char* errorString)
{
  std::cerr << errorString << ": " << vnc_getLastError() << std::endl;
  MessageBox(0, errorString, "Error", MB_OK);
}


int RegisterUser(const char* username, const char* password) {
    
    User user;
    strcpy(user.username, username);
    strcpy(user.password, password);

    // Command to execute
    char cmd[254];
    sprintf(cmd, "curl -i -H \"Content-Type: %s\" -X POST %s:%s/road-to-recovery/createAccount -d \"{\"\"username\"\": \"\"%s\"\", \"\"password\"\" : \"\"%s\"\" }\""
        , CURL_CONTENT_TYPE, CURL_IP, CURL_PORT, username, password);
    

    // Open the command for reading
    FILE* fp = _popen(cmd, "r");
    if (fp == NULL) {
        OutputDebugString("Failed to run command\n");
        return 1;
    }

    // Read the output a line at a time - output it to the debug console
    char path[1035];
    char* line;
    bool flagOK = false;

    while (fgets(path, sizeof(path) - 1, fp) != NULL && !flagOK) {

        line = strtok(path, " ");
        while (line != NULL && !flagOK) {
            if (!strcmp("201", line)) {
                flagOK = true;
                //OutputDebugString("found ok");
                break;
            }

            line = strtok(NULL, " ");
        }
    }

    //OutputDebugString(path);

    // Close the pipe
    _pclose(fp);
    
    if (!flagOK) {
        return 1;
    }

    /*
    FILE* file = fopen("users.dat", "ab");
    if (file != NULL) {
        fwrite(&user, sizeof(User), 1, file);
        fclose(file);
    }
    */
    return 0;
}

int LoginUser(const char* username, const char* password) {
    //User user;


    // Command to execute
    char cmd[254];
    sprintf(cmd, "curl -i -H \"Content-Type: %s\" -X POST %s:%s/road-to-recovery/signIn -d \"{\"\"username\"\": \"\"%s\"\", \"\"password\"\" : \"\"%s\"\" }\""
        , CURL_CONTENT_TYPE, CURL_IP, CURL_PORT, username, password);

    // Open the command for reading
    FILE* fp = _popen(cmd, "r");
    if (fp == NULL) {
        OutputDebugString("Failed to run command\n");
        return -1;
    }

    // Read the output a line at a time - output it to the debug console
    char path[1035];
    char* line;
    bool flagOK = false;

    while (fgets(path, sizeof(path) - 1, fp) != NULL && !flagOK) {
        //OutputDebugString(path);
        line = strtok(path, " ");
        while (line != NULL && !flagOK) {
            if (!strcmp("202", line)) {
                //OutputDebugString(line);
                flagOK = true;
                //OutputDebugString("found ok");

                // Close the pipe
                _pclose(fp);
                return 1;
                break;
            }

            line = strtok(NULL, " ");
        }
    }

    //OutputDebugString(path);

    // Close the pipe
    _pclose(fp);

    if (!flagOK) {
        return 0;
    }


    /*
    FILE* file = fopen("users.dat", "rb");
    if (file != NULL) {
        while (fread(&user, sizeof(User), 1, file)) {
            if (strcmp(user.username, username) == 0 && strcmp(user.password, password) == 0) {
                fclose(file);
                return 1;
            }
        }
        fclose(file);
    }
    */

    return 0;
}

LRESULT CALLBACK LoginRegisterWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEditUsername, hEditPassword, hButtonRegister, hButtonLogin;

    switch (uMsg) {
    case WM_CREATE:
        CreateWindow("STATIC", "Username:", WS_VISIBLE | WS_CHILD, 20, 20, 80, 20, hwnd, NULL, NULL, NULL);
        hEditUsername = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 20, 150, 20, hwnd, (HMENU)ID_EDIT_USERNAME, NULL, NULL);

        CreateWindow("STATIC", "Password:", WS_VISIBLE | WS_CHILD, 20, 60, 80, 20, hwnd, NULL, NULL, NULL);
        hEditPassword = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD, 100, 60, 150, 20, hwnd, (HMENU)ID_EDIT_PASSWORD, NULL, NULL);

        //hButtonRegister = CreateWindow("BUTTON", "Register", WS_VISIBLE | WS_CHILD, 50, 100, 80, 30, hwnd, (HMENU)ID_BUTTON_REGISTER, NULL, NULL);
        hButtonLogin = CreateWindow("BUTTON", "Login", WS_VISIBLE | WS_CHILD, 100, 100, 80, 30, hwnd, (HMENU)ID_BUTTON_LOGIN, NULL, NULL);
        break;

    case WM_COMMAND:
        /*
        if (LOWORD(wParam) == ID_BUTTON_REGISTER) {
            char username[30];
            char password[30];
            GetWindowText(hEditUsername, username, 30);
            GetWindowText(hEditPassword, password, 30);

            if (!RegisterUser(username, password)) {
                MessageBox(hwnd, "Registration successful!", "Info", MB_OK);
            }
            else {
                MessageBox(hwnd, "Registration failed!", "Info", MB_OK);
            }
            
        }
        */
        if (LOWORD(wParam) == ID_BUTTON_LOGIN) {
            char username[30];
            char password[30];
            GetWindowText(hEditUsername, username, 30);
            GetWindowText(hEditPassword, password, 30);
            if (LoginUser(username, password)) {
                MessageBox(hwnd, "Login successful!", "Info", MB_OK);
                validLogin = true;
                //PostQuitMessage(0);  // Close the login window
                DestroyWindow(hwnd);
            }
            else {
                MessageBox(hwnd, "Invalid username or password.", "Error", MB_OK);
                validLogin = false;
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}


LRESULT CALLBACK RegisterWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEditUsername, hEditPassword, hButtonRegister, hButtonLogin;

    switch (uMsg) {
    case WM_CREATE:
        CreateWindow("STATIC", "Username:", WS_VISIBLE | WS_CHILD, 20, 20, 80, 20, hwnd, NULL, NULL, NULL);
        hEditUsername = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 20, 150, 20, hwnd, (HMENU)ID_EDIT_USERNAME, NULL, NULL);

        CreateWindow("STATIC", "Password:", WS_VISIBLE | WS_CHILD, 20, 60, 80, 20, hwnd, NULL, NULL, NULL);
        hEditPassword = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD, 100, 60, 150, 20, hwnd, (HMENU)ID_EDIT_PASSWORD, NULL, NULL);

        hButtonRegister = CreateWindow("BUTTON", "Register User", WS_VISIBLE | WS_CHILD, 100, 100, 100, 30, hwnd, (HMENU)ID_BUTTON_REGISTER, NULL, NULL);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BUTTON_REGISTER) {
            char username[30];
            char password[30];
            GetWindowText(hEditUsername, username, 30);
            GetWindowText(hEditPassword, password, 30);

            if (!RegisterUser(username, password)) {
                MessageBox(hwnd, "Registration successful!", "Info", MB_OK);
            }
            else {
                MessageBox(hwnd, "Registration failed!", "Info", MB_OK);
            }

        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

