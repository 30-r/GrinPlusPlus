#pragma once

#include <Wallet/Models/Slate/ParticipantData.h>
#include <Crypto/Crypto.h>

class SignatureUtil
{
public:
	static std::unique_ptr<CompactSignature> GeneratePartialSignature(const SecretKey& secretKey, const SecretKey& secretNonce, const std::vector<ParticipantData>& participants, const Hash& message)
	{
		std::vector<PublicKey> pubKeys;
		std::vector<PublicKey> pubNonces;

		for (const ParticipantData& participantData : participants)
		{
			pubKeys.push_back(participantData.GetPublicBlindExcess());
			pubNonces.push_back(participantData.GetPublicNonce());
		}

		const PublicKey sumPubKeys = Crypto::AddPublicKeys(pubKeys);
		const PublicKey sumPubNonces = Crypto::AddPublicKeys(pubNonces);

		return Crypto::CalculatePartialSignature(secretKey, secretNonce, sumPubKeys, sumPubNonces, message);
	}

	static std::unique_ptr<Signature> AggregateSignatures(const std::vector<ParticipantData>& participants)
	{
		std::vector<CompactSignature> signatures;
		std::vector<PublicKey> pubNonces;

		for (const ParticipantData& participantData : participants)
		{
			pubNonces.push_back(participantData.GetPublicNonce());

			if (participantData.GetPartialSignature().has_value())
			{
				signatures.push_back(*participantData.GetPartialSignature());
			}
		}

		const PublicKey sumPubNonces = Crypto::AddPublicKeys(pubNonces);

		return Crypto::AggregateSignatures(signatures, sumPubNonces);
	}

	static bool VerifyPartialSignatures(const std::vector<ParticipantData>& participants, const Hash& message)
	{
		std::vector<PublicKey> pubKeys;
		std::vector<PublicKey> pubNonces;

		for (const ParticipantData& participantData : participants)
		{
			pubKeys.push_back(participantData.GetPublicBlindExcess());
			pubNonces.push_back(participantData.GetPublicNonce());
		}

		const PublicKey sumPubKeys = Crypto::AddPublicKeys(pubKeys);
		const PublicKey sumPubNonces = Crypto::AddPublicKeys(pubNonces);

		for (const ParticipantData& participant : participants)
		{
			if (participant.GetPartialSignature().has_value())
			{
				if (!Crypto::VerifyPartialSignature(*participant.GetPartialSignature(), participant.GetPublicBlindExcess(), sumPubKeys, sumPubNonces, message))
				{
					return false;
				}
			}
		}

		return true;
	}

	static bool VerifyMessageSignatures(const std::vector<ParticipantData>& participants)
	{
		for (const ParticipantData& participant : participants)
		{
			if (participant.GetMessageText().has_value())
			{
				// TODO: Limit message length
				if (!Crypto::VerifyMessageSignature(*participant.GetMessageSignature(), participant.GetPublicBlindExcess(), *participant.GetMessageText()))
				{
					return false;
				}
			}
		}

		return true;
	}

	static bool VerifyAggregateSignature(const Signature& aggregateSignature, const std::vector<ParticipantData>& participants, const Hash& message)
	{
		std::vector<PublicKey> pubKeys;

		for (const ParticipantData& participantData : participants)
		{
			pubKeys.push_back(participantData.GetPublicBlindExcess());
		}

		const PublicKey sumPubKeys = Crypto::AddPublicKeys(pubKeys);

		return Crypto::VerifyAggregateSignature(aggregateSignature, sumPubKeys, message);
	}
};