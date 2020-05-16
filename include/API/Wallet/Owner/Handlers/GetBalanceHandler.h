#pragma once

#include <Config/Config.h>
#include <Wallet/WalletManager.h>
#include <Net/Clients/RPC/RPC.h>
#include <Net/Servers/RPC/RPCMethod.h>
#include <API/Wallet/Owner/Models/Errors.h>
#include <optional>

class GetBalanceHandler : public RPCMethod
{
public:
	GetBalanceHandler(const IWalletManagerPtr& pWalletManager)
		: m_pWalletManager(pWalletManager) { }
	virtual ~GetBalanceHandler() = default;

	RPC::Response Handle(const RPC::Request& request) const final
	{
		if (!request.GetParams().has_value())
		{
			return request.BuildError(RPC::Errors::PARAMS_MISSING);
		}

		Json::Value tokenJson = JsonUtil::GetRequiredField(*request.GetParams(), "session_token");
		SessionToken token = SessionToken::FromBase64(tokenJson.asString());

		auto balance = m_pWalletManager->GetBalance(token);

		return request.BuildResult(balance.ToJSON());
	}

	bool ContainsSecrets() const noexcept final { return false; }

private:
	IWalletManagerPtr m_pWalletManager;
};