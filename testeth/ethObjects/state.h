#pragma once
#include <testeth/DataObject.h>
#include <testeth/ethObjects/account.h>

namespace test {
	class state
	{
		public:
		state(DataObject const& _state)
		{
			for (auto const& accountObj : _state.getSubObjects())
				m_accounts.push_back(account(accountObj));
		}
		private:
		std::vector<account> m_accounts;
	};
}

