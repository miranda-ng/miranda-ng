#ifndef _STEAM_AVATAR_H_
#define _STEAM_AVATAR_H_

namespace SteamWebApi
{
	class AvatarApi : public BaseApi
	{
	public:

		class Avatar : public Result
		{
			friend AvatarApi;

		private:
			size_t size;
			BYTE *data;

		public:
			Avatar() : size(0), data(NULL) { }
			~Avatar()
			{
				if (data != NULL)
					mir_free(data);
			}

			size_t GetDataSize() const { return size; }
			const BYTE * GetData() const { return data; }
		};

		static void GetAvatar(HANDLE hConnection, const char *avatarUrl, Avatar *avatar)
		{
			avatar->success = false;

			/*HttpGetRequest request(hConnection, avatarUrl);
			request.ResetFlags(NLHRF_HTTP11 | NLHRF_NODUMP);

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response)
				return;

			if ((avatar->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK)
				return;

			avatar->size = response->dataLength;
			avatar->data = (BYTE*)mir_alloc(avatar->size);
			memcpy(avatar->data, response->pData, avatar->size);*/

			avatar->success = true;
		}
	};
}


#endif //_STEAM_AVATAR_H_