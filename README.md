<p align="center">
  <img src="https://github.com/larty77/ICENet/assets/125078218/c28309e2-377a-450f-9440-8b7e8eaf335a" alt="logo" width="200" height="200">
</p>

<h1 align="center" tabindex="-1" dir="auto"><a class="anchor" aria-hidden="true"></a>ICENet</h1>

Use it in <a href = "https://github.com/larty77/ice_net">ice_net</a>

Client:

```cpp
void start()
{
	client.connect<win_udp_client>(end_point("127.0.0.1", 7777), end_point(0, 0));
	std::thread tick_t([&]() { while (true) client.update(); });

	client.connected_callback = [this, &client]()
	{
		ice_data::write data;

		data.add_string("Hello World!");

		client.send_reliable(data);
	};

	tick_t.join();
}
```

Server:

```cpp
void start()
{
	rudp_server server;
	server.try_start<win_udp_server>(end_point(0, 7777));
	std::thread tick_t([&]() { while (true) server.update(); });

	server.external_data_callback = [this](rudp_connection& c, ice_data::read& d) 
	{
		handle(c, d); 
	};

	tick_t.join();
}

void handle(rudp_connection& c, ice_data::read& data)
{
	std::cout << c.get_remote_point().get_port_str() << ": " << data.get_string() << "\n";
}
```
