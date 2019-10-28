#ifndef GRADIDO_LOGIN_SERVER_TASKS_SIGNING_TRANSACTION_INCLUDE 
#define GRADIDO_LOGIN_SERVER_TASKS_SIGNING_TRANSACTION_INCLUDE

#include "CPUTask.h"

#include "../model/ErrorList.h"
#include "../model/TransactionBase.h"

#include "../proto/gradido/Transaction.pb.h"

#include "ProcessingTransaction.h"

/*
* @author: Dario Rekowski
*
* @date: 28.10.19
* @desc: Task for signing Transactions
*/

class SigningTransaction : public UniLib::controller::CPUTask, public ErrorList
{
public:
	SigningTransaction(Poco::AutoPtr<ProcessingTransaction> transactionBody);
	virtual ~SigningTransaction();

	int run();

	const char* getResourceType() const { return "SigningTransaction"; };

	

protected:
	Poco::AutoPtr<ProcessingTransaction> mTransactionBody;
	
private:

};


#endif //GRADIDO_LOGIN_SERVER_TASKS_SIGNING_TRANSACTION_INCLUDE