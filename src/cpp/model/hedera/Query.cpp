#include "Query.h"
#include "QueryHeader.h"
#include "Poco/Timestamp.h"
#include "../../SingletonManager/MemoryManager.h"

#include "Transaction.h"
#include "TransactionBody.h"
#include "CryptoTransferTransaction.h"

namespace model {
	namespace hedera {

		Query::Query()
			: mTransactionBody(nullptr)
		{

		}

		Query::~Query()
		{
			if (mTransactionBody) {
				delete mTransactionBody;
			}
		}

		Query* Query::getBalance(Poco::AutoPtr<controller::HederaId> accountId, const controller::NodeServerConnection& connection)
		{
		
			assert(!accountId.isNull() && accountId->getModel());

			printf("[Query::getBalance] account id: %s\n", accountId->getModel()->toString().data());

			auto query = new Query;
			auto get_account_balance = query->mQueryProto.mutable_cryptogetaccountbalance();
			accountId->copyToProtoAccountId(get_account_balance->mutable_accountid());
			auto query_header = get_account_balance->mutable_header();
			query_header->set_responsetype(proto::COST_ANSWER);

			query->mTransactionBody = new TransactionBody(accountId, connection);
			CryptoTransferTransaction crypto_transaction;
			crypto_transaction.addSender(accountId, 0);
			crypto_transaction.addReceiver(connection.hederaId, 0);
			query->mTransactionBody->setCryptoTransfer(crypto_transaction);

			//auto transaction = query_header->mutable_payment();
			//auto transaction_body = transaction->mutable_body();
			// body content
			// node account id
			

			return query;
		}

		Query* Query::getTopicInfo(Poco::AutoPtr<controller::HederaId> topicId, Poco::AutoPtr<controller::HederaId> payerAccountId, const controller::NodeServerConnection& connection)
		{
			assert(!topicId.isNull() && topicId->getModel());
			assert(!payerAccountId.isNull() && payerAccountId->getModel());

			printf("[Query::getBalance] topic id: %s\n", topicId->getModel()->toString().data());
			printf("[Query::getBalance] payer account id: %s\n", payerAccountId->getModel()->toString().data());

			auto query = new Query;
			auto get_topic_info = query->mQueryProto.mutable_consensusgettopicinfo();
			topicId->copyToProtoTopicId(get_topic_info->mutable_topicid());

			auto query_header = get_topic_info->mutable_header();
			query_header->set_responsetype(proto::ANSWER_ONLY);

			query->mTransactionBody = new TransactionBody(payerAccountId, connection);
			CryptoTransferTransaction crypto_transaction;
			// 0.003317 Hashbars
			// fee from https://www.hedera.com/fees
			crypto_transaction.addSender(payerAccountId, 3317);
			crypto_transaction.addReceiver(connection.hederaId, 3317);
			query->mTransactionBody->setCryptoTransfer(crypto_transaction);

			return query;
		}

		Query* Query::getTransactionGetReceiptQuery(
			const proto::TransactionID& transactionId,
			Poco::AutoPtr<controller::HederaAccount> payerAccount,
			const controller::NodeServerConnection& connection
		)
		{
			assert(!payerAccount.isNull());
			auto query = new Query;
			query->mQueryHeader = QueryHeader::createWithPaymentTransaction(payerAccount, connection, 0);
			auto transaction_get_receipt_query = query->mQueryProto.mutable_transactiongetreceipt();
			transaction_get_receipt_query->set_allocated_header(query->mQueryHeader->getProtoQueryHeader());
			auto transaction_id = transaction_get_receipt_query->transactionid();
			transaction_id = transactionId;

			return query;
		}

		proto::QueryHeader* Query::getQueryHeader()
		{
			if (mQueryProto.has_cryptogetaccountbalance()) {
				return mQueryProto.mutable_cryptogetaccountbalance()->mutable_header();
			}
			else if (mQueryProto.has_consensusgettopicinfo()) {
				return mQueryProto.mutable_consensusgettopicinfo()->mutable_header();
			}
			else {
				return mQueryHeader->getProtoQueryHeader();
			}
			return nullptr;
		}

		bool Query::sign(std::unique_ptr<KeyPairHedera> keyPairHedera)
		{
			Transaction transaction;
			mTransactionBody->updateTimestamp();
			auto sign_result = transaction.sign(std::move(keyPairHedera), mTransactionBody);
			auto query_header = getQueryHeader();
			query_header->set_allocated_payment(transaction.getTransaction());
			transaction.resetPointer();

			return sign_result;
		}

		void Query::setResponseType(proto::ResponseType type)
		{
			auto query_header = getQueryHeader();
			query_header->set_responsetype(type);
		}

		proto::ResponseType Query::getResponseType()
		{
			auto query_header = getQueryHeader();
			return query_header->responsetype();
		}

		
	}
}