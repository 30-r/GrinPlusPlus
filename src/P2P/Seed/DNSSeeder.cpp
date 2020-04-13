#include "DNSSeeder.h"

#include <Infrastructure/Logger.h>
#include <asio.h>

DNSSeeder::DNSSeeder(const Config& config)
	: m_config(config)
{

}

std::vector<SocketAddress> DNSSeeder::GetPeersFromDNS() const
{
	std::vector<SocketAddress> addresses;

	std::vector<std::string> dnsSeeds;
	if (m_config.GetEnvironment().IsMainnet())
	{
		dnsSeeds = {
			"mainnet.seed.grin-tech.org",		// igno.peverell@protonmail.com
			"mainnet.seed.grin.icu",			// gary.peverell@protonmail.com
			"mainnet.seed.713.mw",				// jasper@713.mw
			"mainnet.seed.grin.lesceller.com",	// q.lesceller@gmail.com
			"mainnet.seed.grin.prokapi.com",	// hendi@prokapi.com
			"grinseed.yeastplume.org",			// yeastplume@protonmail.com
		};
	}
	else
	{
		dnsSeeds = {
			"floonet.seed.grin-tech.org",		// igno.peverell@protonmail.com
			"floonet.seed.grin.icu",			// gary.peverell@protonmail.com
			"floonet.seed.713.mw",				// jasper@713.mw
			"floonet.seed.grin.lesceller.com",	// q.lesceller@gmail.com
			"floonet.seed.grin.prokapi.com",	// hendi@prokapi.com
		};
	}

	for (auto seed : dnsSeeds)
	{
		LOG_TRACE_F("Checking seed: {}", seed);
		const std::vector<IPAddress> ipAddresses = Resolve(seed);
		for (const IPAddress ipAddress : ipAddresses)
		{
			LOG_TRACE_F("IP Address: {}", ipAddress);
			addresses.emplace_back(SocketAddress(ipAddress, m_config.GetEnvironment().GetP2PPort()));
		}

		if (!m_config.GetEnvironment().IsMainnet())
		{
			addresses.emplace_back(SocketAddress("100.26.68.39", 13414));
		}
	}

	return addresses;
}

std::vector<IPAddress> DNSSeeder::Resolve(const std::string& domainName) const
{
	asio::io_context context;
	asio::ip::tcp::resolver resolver(context);
	asio::ip::tcp::resolver::query query(domainName, "domain");
	asio::error_code errorCode;
	asio::ip::tcp::resolver::iterator iter = resolver.resolve(query, errorCode);

	std::vector<IPAddress> addresses;
	if (!errorCode)
	{
		std::for_each(iter, {}, [&addresses](auto& it)
			{
				try
				{
					addresses.push_back(IPAddress(it.endpoint().address()));
				}
				catch (std::exception& e)
				{
					LOG_INFO_F("Exception thrown: {}", e.what());
				}
			});
	}
	else
	{
		LOG_TRACE_F("Error: {}", errorCode.message());
	}
	

	return addresses;
}