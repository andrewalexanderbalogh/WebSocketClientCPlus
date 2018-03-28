/* standard libs */
#include <sstream>
#include <iostream>
#include <algorithm>
#include <memory>
#include <map>
#include <random>
#include <string>
#include <thread>

/* easywsclient lib: https://github.com/dhbaird/easywsclient.git */
#include "../third_party/easywsclient/easywsclient.hpp"
#include "../third_party/easywsclient/easywsclient.cpp"

/* rapidjson lib: https://github.com/Tencent/rapidjson.git */
#include "../third_party/rapidjson/document.h"
#include "../third_party/rapidjson/writer.h"

#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif

using std::string;
using std::stringstream;
using std::cout;
using std::cin;
using std::endl;
using std::thread;
using std::unique_ptr;
using std::shared_ptr;
using std::pair;

using easywsclient::WebSocket;
using rapidjson::Document;
using rapidjson::StringBuffer;
using rapidjson::Writer;
using rapidjson::Value;

/* Control constants that define the context of the message sent */
const char* const RESET_SERVER = "RESET_SERVER";
const char* const SEND_DATA = "SEND_DATA";
const char* const BROADCAST = "BROADCAST";
const char* const INIT_CLIENT = "INIT_CLIENT";
const char* const TARGET_CLIENT = "TARGET_CLIENT";

/* Map control-string constants to shorts, so can use case statement to determine client responses to messages */
std::map<string, short> typeMap;

/* Address of the WebSocket Server we want to connect with */
const char* const WS_SERVER_ADDR = "ws://192.168.1.150:8080";

/* Setup WebSocket server pointer as a global */
shared_ptr<WebSocket> ws;

/* Global to indicate to event-loop threads that they should return */
bool shut_down_thread = false;

/**Handler for incoming messages from WebSocket Server
 * @param message
 */
void handle_message(const string& message) {
    Document jsonParser;
    jsonParser.Parse(message.c_str());

    /* Failure if we cant parse the message from the server */
    assert(jsonParser.IsObject());

    cout << endl << "-----MESSAGE FROM WEBSOCKET SERVER-----" << endl;

    const Value& type = jsonParser["type"];
    const Value& content = jsonParser["content"];

    short typeValue = typeMap.find(type.GetString())->second;
    switch (typeValue){
        case 3:                 // INIT_CLIENT
            cout << endl << "***CONNECTED CLIENTS***" << endl;
            /* iterate through array of connected clients */
            for(auto& client: content.GetArray()){
                cout << endl;
                for(auto& stats: client.GetObject()){
                    cout << stats.name.GetString() <<  "->" <<  stats.value.GetString() << endl;
                }
            }
            cout << endl << "***********************" << endl;
            break;
        case 1:                 // SEND_DATA
            /* falls through */
        case 2:                 // BROADCAST
            /* falls through */
        case 4:                 // TARGET_CLIENT
            /* falls through */
        default:
            cout << content.GetString() << endl;
            break;
    }

    cout << "---------------------------------------" << endl << endl;
}


/**Create a JSON string composite message to send off to WebSocket Server
 * @param control_message
 * @return string
 */
string write_message(const char* const type, const string& status = string(""), const string& content = string(""), const string& target = string("")){
    StringBuffer jsonBuff;
    Writer<StringBuffer> jsonWriter(jsonBuff);

    jsonWriter.StartObject();
    jsonWriter.Key("type");         // output a key,
    jsonWriter.String(type);        // follow by a value.

    if (!status.empty()){
        jsonWriter.Key("status");
        jsonWriter.String(status.c_str());
    }
    if (!content.empty()){
        jsonWriter.Key("content");
        jsonWriter.String(content.c_str());
    }
    if (!target.empty()){
        jsonWriter.Key("target");
        jsonWriter.String(target.c_str());
    }


    jsonWriter.EndObject();

    return string(jsonBuff.GetString());
}


/**Generate some random value to give us a default identifier
 * @return string
 */
string get_uuid(){
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(1,10000);

    string uuid;
    stringstream stringCat;
    stringCat << "VAC_CPP" << dist(mt);
    stringCat >> uuid;

    return uuid;
}


/**Startup the WebSocket client, and open event loop for receiving messages back from server
 */
void websocket_event_loop(){
    /* Complete initialization of mapping control-string constants to shorts */
    typeMap.insert(pair<string, int>(RESET_SERVER, 0));
    typeMap.insert(pair<string, int>(SEND_DATA, 1));
    typeMap.insert(pair<string, int>(BROADCAST, 2));
    typeMap.insert(pair<string, int>(INIT_CLIENT, 3));
    typeMap.insert(pair<string, int>(TARGET_CLIENT, 4));


    /* Build the initial json message we will send to the WebSocket server as a handshake */
    string jsonMessage = write_message(INIT_CLIENT, "LOVELY", get_uuid());
    ws->send(jsonMessage);


    while (ws->getReadyState() != WebSocket::CLOSED && !shut_down_thread) {
        ws->poll(50);   // introduce 50ms timeout between polls to message-buffer, to avoid potential "RangeError: Invalid WebSocket frame: invalid opcode 0" issue at server
        ws->dispatch(handle_message);
    }

    if (shut_down_thread){
        return;
    }

    cout << "!!! WebSocket Connection terminated at server !!!" << endl;
    ws->close();

    /* We will automatically try to reconnect to the server every 5000ms, by swapping out a new connection to the server, and recursively calling websocket_event_loop */
    cout << "!!! Attempting to reconnect to server !!!" << endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    shared_ptr<WebSocket> new_ws(WebSocket::from_url(WS_SERVER_ADDR));
    ws.reset();
    ws.swap(new_ws);

    websocket_event_loop();
}


/**Main method to run c++ WebSocket Client
 * @return
 */
int main() {
#ifdef _WIN32
    INT rc;
    WSADATA wsaData;

    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc) {
        printf("WSAStartup Failed.\n");
        return 1;
    }
#endif

    /* Setup our WebSocket Server address */
    shared_ptr<WebSocket> new_ws(WebSocket::from_url(WS_SERVER_ADDR));
    ws.swap(new_ws);
    assert(ws);

    /* Start and run our WebSocket-Client in a separate thread */
    thread wsThread(websocket_event_loop);;

    /* Run interactive console program in main thread to send additional types of messages to the server */
    string userMsg;
    string msgType = SEND_DATA;
    string targetClient;
    string jsonMsg;
    short msgContext = 1;
    while(msgContext != 99){

        /* Make sure we got a connected client. Else just sit. */
        if (ws->getReadyState() == WebSocket::CLOSED){
            cout << "# Disconnected from server, please wait as we try to reconnect.." << endl;
            while (ws->getReadyState() == WebSocket::CLOSED){
                std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            }
            cout << "# Successfully reconnected to server." << endl;
            continue;
        }


        cout << "# What kind of message to send to server?:" << endl;
        cout << "# 1->SEND_DATA, 2->BROADCAST, 4->TARGET_CLIENT, 9->RESET_SERVER.  Enter '99' to Exit program" << endl;
        cin >> msgContext;

        switch (msgContext){
            case 1:                 // SEND_DATA
                msgType = SEND_DATA;
                targetClient = "";
                break;
            case 2:                 // BROADCAST
                msgType = BROADCAST;
                targetClient = "";
                break;
            case 4:                 // TARGET_CLIENT
                msgType = TARGET_CLIENT;
                cout << "# Which client to send message to?:" << endl;
                cin >> targetClient;
                break;
            case 9:                 // RESET_SERVER
                msgType = RESET_SERVER;
                userMsg = "";
                targetClient = "";
                cout << "# Sending reset signal to server:" << endl;
                break;
            case 99:
                cout << "Shutting down client and exiting program." << endl;
                break;
            default:
                cout << "# Choice not recognized, starting over.." << endl;
                msgContext = 0;
                break;
        }


        if (msgContext == 9){
            jsonMsg = write_message(msgType.c_str());
            ws->send(jsonMsg);
        }
        else if (msgContext == 99){
            break;
        }
        else if (msgContext) {
            cout << "# Enter message content to send:" << endl;
            cin >> userMsg;
            jsonMsg = write_message(msgType.c_str(), "LOVELY", userMsg, targetClient);
            // cout << jsonMsg << endl;
            ws->send(jsonMsg);
        }
    }

    /* Properly close client connection before exiting */
    shut_down_thread = true;
    ws->close();
    wsThread.join();

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
