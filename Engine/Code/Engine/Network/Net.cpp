#include "Engine/Network/Net.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Network/TCPSocket.hpp"
#include "Engine/Network/NetAddress.hpp"
#include "Engine/Core/XMLUtils.hpp"


static std::string s_serviceType;
static std::string s_controlURL;
static std::string s_externalIP;
static std::string s_infoURL;

bool NetSystemStartup()
{
	//Pick WInsock Version
	WORD version = MAKEWORD(2, 2);

	WSADATA data;
	int error = ::WSAStartup(version, &data);
	return error == 0;
}

void NetSystemShutdown()
{
	::WSACleanup();

	s_serviceType.clear();
	s_controlURL.clear();
	s_externalIP.clear();
	s_infoURL.clear();
}

// Search Info
#define SEARCH_REQUEST_STRING "M-SEARCH * HTTP/1.1\r\n" "ST:UPnP:rootdevice\r\n" "MX: 3\r\n" "Man:\"ssdp:discover\"\r\n" "HOST: 239.255.255.250:1900\r\n" "\r\n"
#define MAX_BUFF_SIZE 102400
#define HTTPMU_HOST_ADDRESS "239.255.255.250"
#define HTTPMU_HOST_PORT 1900

// Device Setting Info
#define DEVICE_TYPE_1	"urn:schemas-upnp-org:device:InternetGatewayDevice:1"
#define DEVICE_TYPE_2	"urn:schemas-upnp-org:device:WANDevice:1"
#define DEVICE_TYPE_3	"urn:schemas-upnp-org:device:WANConnectionDevice:1"
#define SERVICE_WANIP	"urn:schemas-upnp-org:service:WANIPConnection:1"
#define SERVICE_WANPPP	"urn:schemas-upnp-org:service:WANPPPConnection:1" 

// HTTP Information
#define HTTP_HEADER_ACTION "POST %s HTTP/1.1\r\n" "HOST: %s:%u\r\n" "SOAPACTION:\"%s#%s\"\r\n" "CONTENT-TYPE: text/xml ; charset=\"utf-8\"\r\n" "Content-Length: %d \r\n\r\n"
#define SOAP_ACTION "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" "<s:Envelope xmlns:s=" "\"http://schemas.xmlsoap.org/soap/envelope/\" " "s:encodingStyle=" "\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n" "<s:Body>\r\n" "<u:%s xmlns:u=\"%s\">\r\n%s" "</u:%s>\r\n" "</s:Body>\r\n" "</s:Envelope>\r\n"
#define PORT_MAPPING_LEASE_TIME "86400"    //One Day Lease Time In Seconds 
#define ADD_PORT_MAPPING_PARAMS "<NewRemoteHost></NewRemoteHost>\r\n" "<NewExternalPort>%u</NewExternalPort>\r\n" "<NewProtocol>%s</NewProtocol>\r\n" "<NewInternalPort>%u</NewInternalPort>\r\n" "<NewInternalClient>%s</NewInternalClient>\r\n" "<NewEnabled>1</NewEnabled>\r\n" "<NewPortMappingDescription>%s</NewPortMappingDescription>\r\n" "<NewLeaseDuration>" PORT_MAPPING_LEASE_TIME "</NewLeaseDuration>\r\n"
#define ACTION_ADD "AddPortMapping"
#define ACTION_GET_EXT_IP "GetExternalIPAddress"
#define HTTP_OK "200 OK"
#define SOAP_ACTION_EXTERNAL_IP  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" "<s:Envelope xmlns:s=" "\"http://schemas.xmlsoap.org/soap/envelope/\" " "s:encodingStyle=" "\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n" "<s:Body>\r\n" "<u:%s xmlns:u=\"%s\">\r\n" "</u:%s>\r\n" "</s:Body>\r\n" "</s:Envelope>\r\n"
#define HTTP_HEADER_BASIC "POST %s HTTP/1.1\r\n" "HOST: %s:%u\r\n" "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n" "Content-Length: %d \r\n\r\n"

std::string GetLastErrorAsString()
{
	DWORD errorMessageID = ::WSAGetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	::LocalFree(messageBuffer);

	return message;
}

bool ParseUrl(const std::string& url, std::string& host_ip, unsigned short int* host_port, std::string& host_addr)
{
	std::string::size_type begin_domain;
	std::string::size_type end_ip;
	std::string::size_type end_domain;

	begin_domain = url.find("://");
	if (begin_domain == std::string::npos)
	{
		DebuggerPrintf("URL %s is improperly formatted at the domain head!\n", url.c_str());
		return false;
	}
	begin_domain = begin_domain + 3;

	end_ip = url.find(":", begin_domain);
	if (end_ip == std::string::npos)
	{
		*host_port = 80;
		end_domain = url.find("/", begin_domain);
		if (end_domain == std::string::npos)
		{
			DebuggerPrintf("URL %s is improperly formatted at the domain end!\n", url.c_str());
			return false;
		}

		host_ip = url.substr(begin_domain, end_domain - begin_domain);
	}
	else
	{
		host_ip = url.substr(begin_domain, end_ip - begin_domain);
		end_domain = url.find("/", begin_domain);
		if (end_domain == std::string::npos)
		{
			DebuggerPrintf("URL %s is improperly formatted at the domain end!\n", url.c_str());
			return false;
		}

		std::string str_port = url.substr(end_ip + 1, end_domain - end_ip - 1);
		*host_port = (unsigned short)atoi(str_port.c_str());
	}

	if (end_domain + 1 >= url.size())
	{
		host_addr = "/";
	}
	else
	{
		host_addr = url.substr(end_domain, url.size());
	}

	return true;
}

bool GetUPNPDescription(const std::string& description_url, std::string& description_info, TCPSocket** tcp_socket)
{
	std::string host_ip;
	std::string host_addr;
	unsigned short int host_port;

	// We Must Parse the returned URL for the IP, PORT, and combination of the two
	bool error = ParseUrl(description_url, host_ip, &host_port, host_addr);
	if (!error)
	{
		DebuggerPrintf("Failed to parse URL at: %s\n", description_url.c_str());
		return false;
	}

	// Create TCP Socket to send HTTP Request Through
	*tcp_socket = new TCPSocket();
	std::string domain_str = host_ip + ":" + std::to_string(host_port);
	net_address_t tcp_addr = StringToNetAddress(domain_str);
	if (!(*tcp_socket)->Join(tcp_addr))
	{
		delete *tcp_socket;
		DebuggerPrintf("Failed to to establish TCP Socket for Domain: %s\n", domain_str.c_str());
		return false;
	}

	// Create HTTP Request
	char request[200];
	sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", host_addr.c_str(), domain_str.c_str());
	std::string http_request = request;

	// Send Request over TCP Socket
	int return_val = ::send((SOCKET)(*tcp_socket)->m_socket, http_request.c_str(), http_request.size(), 0);
	if (return_val == SOCKET_ERROR)
	{
		std::string msg = GetLastErrorAsString();
		ASSERT_OR_DIE(false, msg.c_str());
	}

	// buffer to recieve description xml file
	char xml_buffer[MAX_BUFF_SIZE + 1];
	::memset(xml_buffer, 0, sizeof(xml_buffer));

	// Recieving loop that writes buffer to info string
	while (return_val = recv((SOCKET)(*tcp_socket)->m_socket, xml_buffer, MAX_BUFF_SIZE, 0) > 0)
	{
		description_info += xml_buffer;
		::memset(xml_buffer, 0, sizeof(xml_buffer));
	}

	std::string::size_type begin_xml = description_info.find("<?xml");
	description_info.erase(0, begin_xml);

	return true;
}

bool ParseDescriptionInfo(const std::string& description_url, const std::string& description_info, TCPSocket** tcp_socket)
{
	// Establish if info is parsable XML
	tinyxml2::XMLDocument xml_doc;
	int xml_error = xml_doc.Parse(description_info.c_str());
	if (xml_error != 0)
	{
		DebuggerPrintf("TinyXML2 has exited with error code: &i\n", xml_error);
		return false;
	}

	tinyxml2::XMLNode* header = xml_doc.FirstChild();
	if (!header)
	{
		DebuggerPrintf("Recieved improperly formatted XML without header\n");
		return false;
	}

	tinyxml2::XMLNode* root = xml_doc.FirstChildElement("root");
	if (!root)
	{
		DebuggerPrintf("Recieved improperly formatted XML without root\n");
		return false;
	}

	tinyxml2::XMLElement* base_url_elem = root->FirstChildElement("URLBase");
	std::string base_url;
	// Required check since base url is not a required node
	if (!base_url_elem)
	{
		std::string::size_type end_index = description_url.find("/", 7);
		if (end_index == std::string::npos)
		{
			DebuggerPrintf("Failed to get Base Url from XML or URL Description!\n");
			return false;
		}

		base_url.assign(description_url, 0, end_index);
	}
	else
	{
		base_url = base_url_elem->GetText();
		if (base_url.empty())
		{
			DebuggerPrintf("Base URL is empty!\n");
			return false;
		}
	}

	// Check physical device information
	tinyxml2::XMLElement* phys_device_elem = root->FirstChildElement("device");
	if (!phys_device_elem)
	{
		DebuggerPrintf("Physical Device info missing from xml!\n");
		return false;
	}
	tinyxml2::XMLElement* phys_device_type_elem = phys_device_elem->FirstChildElement("deviceType");
	if (!phys_device_type_elem)
	{
		DebuggerPrintf("Physical Device Type info missing from xml!\n");
		return false;
	}

	std::string type_info = phys_device_type_elem->GetText();
	if (type_info.empty() || ::strcmp(type_info.c_str(), DEVICE_TYPE_1) != 0)
	{
		DebuggerPrintf("Failed to find device with type '%s' \n", DEVICE_TYPE_1);
		return false;
	}

	// get List of Virtual Devices
	tinyxml2::XMLElement* phys_device_list_elem = phys_device_elem->FirstChildElement("deviceList");
	if (!phys_device_list_elem)
	{
		DebuggerPrintf("Failed to find device list of device '%s' \n", DEVICE_TYPE_1);
		return false;
	}

	// Loop through Virtual list becasue each virtual device can have a list of its own
	// Exits when a device of type 2 has a child of type 3
	// the type 3 child is one that meets all the requirements to support upnp
	tinyxml2::XMLElement* upnp_device_elem = nullptr;
	for (tinyxml2::XMLElement* virt_device_elem = phys_device_list_elem->FirstChildElement("device"); virt_device_elem != nullptr && upnp_device_elem == nullptr; virt_device_elem = virt_device_elem->NextSiblingElement("device"))
	{
		tinyxml2::XMLElement* virt_device_type_elem = virt_device_elem->FirstChildElement("deviceType");
		if (!virt_device_type_elem)
		{
			continue;
		}

		// Crawl nodes for type 2 device
		std::string type_info = virt_device_type_elem->GetText();
		if (type_info.empty() || ::strcmp(type_info.c_str(), DEVICE_TYPE_2) != 0)
		{
			continue;
		}
		else if (::strcmp(type_info.c_str(), DEVICE_TYPE_2) == 0)
		{
			tinyxml2::XMLElement* virt_device_list_elem = virt_device_elem->FirstChildElement("deviceList");
			if (!virt_device_list_elem)
			{
				if (!virt_device_elem->NextSiblingElement("device"))
				{
					virt_device_elem = virt_device_elem->Parent()->ToElement();
				}

				continue;
			}
			tinyxml2::XMLElement* potential_virt_device = virt_device_list_elem->FirstChildElement("device");
			if (potential_virt_device)
			{
				// step down list
				virt_device_elem = potential_virt_device;
			}
			else
			{
				// go to parents next if no device found
				virt_device_elem = virt_device_list_elem->Parent()->ToElement();
			}
		}

		//Crawl through new list for device type 3
		tinyxml2::XMLElement* new_virt_device_type_elem = virt_device_elem->FirstChildElement("deviceType");
		if (!new_virt_device_type_elem)
		{
			if (!virt_device_elem->NextSiblingElement("device"))
			{
				virt_device_elem = virt_device_elem->Parent()->Parent()->ToElement();
			}

			continue;
		}

		type_info = new_virt_device_type_elem->GetText();
		if (!type_info.empty() && ::strcmp(type_info.c_str(), DEVICE_TYPE_3) == 0)
		{
			upnp_device_elem = virt_device_elem;
		}
		else
		{
			if (!virt_device_elem->NextSiblingElement("device"))
			{
				virt_device_elem = virt_device_elem->Parent()->Parent()->ToElement();
			}
		}
	}

	if (!upnp_device_elem)
	{
		DebuggerPrintf("Failed to find device with either type '%s' \n or type '%s' \n", DEVICE_TYPE_2, DEVICE_TYPE_3);
		return false;
	}

	tinyxml2::XMLElement* service_list_elem = upnp_device_elem->FirstChildElement("serviceList");
	if (!service_list_elem)
	{
		DebuggerPrintf("Failed to find service list of device '%s' \n", DEVICE_TYPE_3);
		return false;
	}


	bool is_found = false;
	std::string service_type;
	tinyxml2::XMLElement* correct_service_elem = nullptr;
	for (tinyxml2::XMLElement* service_elem = service_list_elem->FirstChildElement("service"); service_elem != nullptr; service_elem = service_elem->NextSiblingElement("service"))
	{
		tinyxml2::XMLElement* service_type_elem = service_elem->FirstChildElement("serviceType");
		if (!service_type_elem)
		{
			continue;
		}

		service_type = service_type_elem->GetText();
		if (::strcmp(service_type.c_str(), SERVICE_WANIP) == 0 || ::strcmp(service_type.c_str(), SERVICE_WANPPP) == 0)
		{
			correct_service_elem = service_elem;
			is_found = true;
			break;
		}
		else
		{
			service_type.clear();
		}
	}

	if (!is_found)
	{
		DebuggerPrintf("Can't find Service WANIP or WANPP!\n");
		return false;
	}

	s_serviceType = service_type;

	tinyxml2::XMLElement* control_url_elem = correct_service_elem->FirstChildElement("controlURL");
	s_controlURL = control_url_elem->GetText();

	tinyxml2::XMLElement* info_url_elem = correct_service_elem->FirstChildElement("SCPDURL");
	s_infoURL = info_url_elem->GetText();

	// Makes a complete Control Path if not already
	if (s_controlURL.find("http://") == std::string::npos && s_controlURL.find("HTTP://") == std::string::npos)
		s_controlURL = base_url + s_controlURL;

	delete (*tcp_socket);
	return true;
}

bool UPNPDiscovery(uint max_attempts, uint sleep_interval)
{
	// Establishes a UDP socket to broadcast on and discover a router
	SOCKET udp_fwd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//Discovery Protocol for UPNP supported devices on connection
	std::string send_buffer = SEARCH_REQUEST_STRING;

	// Establishes Universal Connection Info for UPNP Multicast Address
	sockaddr_in rec_addr;
	rec_addr.sin_family = AF_INET;
	rec_addr.sin_port = ::htons(HTTPMU_HOST_PORT);
	// Converts String to IN_ADDR Structure
	rec_addr.sin_addr.S_un.S_addr = inet_addr(HTTPMU_HOST_ADDRESS);

	// Sets the socket so it can broadcast to devices and "discover" them
	bool opt_val = true;
	int return_val = ::setsockopt(udp_fwd, SOL_SOCKET, SO_BROADCAST, (char*)&opt_val, sizeof(bool)); // return_val is used continously for error checks
	if (return_val != 0)
	{
		std::string msg = GetLastErrorAsString();
		ASSERT_OR_DIE(false, msg.c_str());
	}

	// Sends specific search string to udp socket to be broadcasted out
	return_val = ::sendto(udp_fwd, send_buffer.c_str(), send_buffer.size(), 0, (sockaddr*)&rec_addr, sizeof(sockaddr_in));
	if (return_val == SOCKET_ERROR)
	{
		std::string msg = GetLastErrorAsString();
		ASSERT_OR_DIE(false, msg.c_str());
	}


	// Buffer needs to be rather large to hold all the information in the protocol
	char rec_buffer[MAX_BUFF_SIZE + 1];

	// string to recieve into
	std::string rec_str;

	// Loop through specified timeout length to check for connection to router
	// Specifically here to establish UPNP before any other connection attempts happen in the game
	for (uint curr = 0; curr < max_attempts; curr++)
	{
		unsigned long blocking_val = 1;
		::ioctlsocket(udp_fwd, FIONBIO, &blocking_val); // Keeps the socket from blocking

		// overwrites memory in buffer
		::memset(rec_buffer, 0, sizeof(rec_buffer));
		//recieves on udp socket
		return_val = ::recvfrom(udp_fwd, rec_buffer, MAX_BUFF_SIZE, 0, NULL, NULL);
		if (return_val == SOCKET_ERROR)
		{
			::Sleep(sleep_interval);
			continue;
		}

		rec_str = rec_buffer;
		return_val = rec_str.find(HTTP_OK);
		// check for reaching the end of an stl string
		if (return_val == std::string::npos)
		{
			// this is an invalid response
			continue;
		}

		// Iterate checks for URL Recieved
		std::string::size_type begin_url = rec_str.find("http://");
		if (begin_url == std::string::npos)
		{
			// this is an invalid response
			continue;
		}

		std::string::size_type end_url = rec_str.find("\r", begin_url);
		if (end_url == std::string::npos)
		{
			// this is an invalid response
			continue;
		}

		// gets url string from
		std::string description_url;
		description_url.assign(rec_str, begin_url, end_url - begin_url);
		// collection of info to parse from Getter
		std::string description_info;
		// TCP Connection to get establish singular connection with the router
		TCPSocket* tcp_socket = nullptr;

		if (!GetUPNPDescription(description_url, description_info, &tcp_socket))
		{
			if(tcp_socket)
				delete tcp_socket;

			::Sleep(sleep_interval);
			continue;
		}

		if (!ParseDescriptionInfo(description_url, description_info, &tcp_socket))
		{
			delete tcp_socket;
			::Sleep(sleep_interval);
			continue;
		}

		::closesocket(udp_fwd);
		return true;
	}

	DebuggerPrintf("Unable To Discover UPnP NAT Device!\n");
	return false;
}

std::string GetExternalIP()
{
	return s_externalIP;
}

void ParseResposeForExternalIP(const std::string& response)
{
	tinyxml2::XMLDocument xml_doc;
	int xml_error = xml_doc.Parse(response.c_str());
	if (xml_error != 0)
	{
		DebuggerPrintf("TinyXML2 has exited with error code: &i\n", xml_error);
		return;
	}

	tinyxml2::XMLNode* root = xml_doc.RootElement();
	if (!root)
	{
		DebuggerPrintf("Recieved improperly formatted XML without root for external IP\n");
		return;
	}

	tinyxml2::XMLElement* body_elem = root->FirstChildElement("SOAP-ENV:Body");
	if (!body_elem)
	{
		DebuggerPrintf("Recieved improperly formatted XML without body for external IP\n");
		return;
	}

	tinyxml2::XMLElement* getter_elem = body_elem->FirstChildElement("u:GetExternalIPAddressResponse");
	if (!getter_elem)
	{
		DebuggerPrintf("Recieved improperly formatted XML without get response for external IP\n");
		return;
	}

	tinyxml2::XMLElement* ext_ip_elem = getter_elem->FirstChildElement("NewExternalIPAddress");
	if (!ext_ip_elem)
	{
		DebuggerPrintf("Recieved improperly formatted XML without external IP\n");
		return;
	}

	s_externalIP = ext_ip_elem->GetText();
}

bool AddPortMapping(const std::string& port_map_name, const std::string& destination_ip, uint16_t external_port, uint16_t internal_port)
{
	// get control url info
	std::string host_ip;
	unsigned short int host_port;
	std::string host_addr;
	bool error = ParseUrl(s_controlURL, host_ip, &host_port, host_addr);
	if (!error)
	{
		DebuggerPrintf("Failed to parse %s\n", s_controlURL.c_str());
		return false;
	}

	TCPSocket* tcp_socket = new TCPSocket();
	std::string domain_str = host_ip + ":" + std::to_string(host_port);
	net_address_t tcp_addr = StringToNetAddress(domain_str);
	if (!tcp_socket->Join(tcp_addr))
	{
		delete tcp_socket;
		DebuggerPrintf("Failed to to establish TCP Socket for Control: %s\n", domain_str.c_str());
		return false;
	}

	tcp_socket->SetBlocking(true);

	// Creates Port Mapping Request
	char buffer[MAX_BUFF_SIZE + 1];
	sprintf(buffer, ADD_PORT_MAPPING_PARAMS, external_port, "TCP", internal_port, destination_ip.c_str(), port_map_name.c_str());
	std::string action_params = buffer;

	sprintf(buffer, SOAP_ACTION, ACTION_ADD, s_serviceType.c_str(), action_params.c_str(), ACTION_ADD);
	std::string soap_message = buffer;

	sprintf(buffer, HTTP_HEADER_ACTION, host_addr.c_str(), host_ip.c_str(), host_port, s_serviceType.c_str(), ACTION_ADD, soap_message.size());
	std::string action_message = buffer;

	std::string http_request = action_message + soap_message;

	int return_val = tcp_socket->Send(http_request.c_str(), http_request.size());
	if (return_val == SOCKET_ERROR)
	{
		delete tcp_socket;
		std::string msg = GetLastErrorAsString();
		ASSERT_OR_DIE(false, msg.c_str());
		return false;
	}

	std::string http_response;
	memset(buffer, 0, sizeof(buffer));
	while (return_val = tcp_socket->Receive(buffer, MAX_BUFF_SIZE) > 0)
	{
		http_response += buffer;
		memset(buffer, 0, sizeof(buffer));
	}

	if (http_response.find(HTTP_OK) == std::string::npos)
	{
		delete tcp_socket;
		DebuggerPrintf("Fail to add port mapping (%s / TCP)\n", port_map_name.c_str());
		return false;
	}

	delete tcp_socket;

	DebuggerPrintf("Sucessfully mapped: %s\n", port_map_name.c_str());

	if (s_externalIP.empty())
	{
		TCPSocket* ip_socket = new TCPSocket();
		if (!ip_socket->Join(tcp_addr))
		{
			delete tcp_socket;
			DebuggerPrintf("Failed to to establish TCP Socket for Control: %s\n", domain_str.c_str());
			return false;
		}

		ip_socket->SetBlocking(true);

		// Create external IP request
		std::string ip_action_params = "<NewExternalIPAddress></NewExternalIPAddress>\r\n";

		sprintf(buffer, SOAP_ACTION, ACTION_GET_EXT_IP, s_serviceType.c_str(), "", ACTION_GET_EXT_IP);
		std::string ip_soap_message = buffer;
	
		sprintf(buffer, HTTP_HEADER_ACTION, host_addr.c_str(), host_ip.c_str(), host_port, s_serviceType.c_str(), ACTION_GET_EXT_IP, ip_soap_message.size());
		std::string ip_action_message = buffer;
	
		std::string ip_http_request = ip_action_message + ip_soap_message;
	
		return_val = ip_socket->Send(ip_http_request.c_str(), ip_http_request.size());
		if (return_val == SOCKET_ERROR)
		{
			delete tcp_socket;
			std::string msg = GetLastErrorAsString();
			ASSERT_OR_DIE(false, msg.c_str());
			return false;
		}
	
		std::string ip_http_response;
		memset(buffer, 0, sizeof(buffer));
		while (return_val = ip_socket->Receive(buffer, MAX_BUFF_SIZE) > 0)
		{
			ip_http_response += buffer;
			memset(buffer, 0, sizeof(buffer));
		}

		delete ip_socket;
	
		std::string::size_type begin_xml = ip_http_response.find("<?xml");
		ip_http_response.erase(0, begin_xml);
	
		ParseResposeForExternalIP(ip_http_response);
	}

	return true;
}

bool EstablishUPNP(const std::string& map_name, uint16_t internal_port, uint16_t external_port, uint max_attempts /*= 10*/, uint sleep_interval /*= 100*/)
{
	if (!UPNPDiscovery(max_attempts, sleep_interval))
	{
		DebuggerPrintf("UPnP Discovery Failed!");
		return false;
	}

	net_address_t my_address = GetMyAddress(internal_port);
	std::string my_ip = NetAddressToStringWithoutPort(my_address);

	if (!AddPortMapping(map_name, my_ip, external_port, internal_port)) 
	{
		DebuggerPrintf("Add Port Mapping Failed!");
		return false;
	}

	return true;
}

std::string GetControlAddress()
{
	std::string host_ip;
	unsigned short int host_port;
	std::string host_addr;
	bool error = ParseUrl(s_controlURL, host_ip, &host_port, host_addr);

	return host_ip + ":" + std::to_string(host_port);
}
