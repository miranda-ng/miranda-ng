/*
* 
*/
#ifndef MEDIAUPLOADER_H_
#define MEDIAUPLOADER_H_

using namespace std;

namespace MediaUploader
{
	std::string pushfile(std::string url, FMessage * message, std::string from);
	std::string getPostString(std::string url, FMessage * message, std::string from);
	std::string sendData(std::string host, std::string head, std::string filePath, std::string tail);
	std::string getExtensionFromMime(string mime);
	std::string getMimeFromExtension(const string &extension);
};

#endif /* MEDIAUPLOADER_H_ */
