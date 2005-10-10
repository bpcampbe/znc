#include "main.h"
#include "User.h"
#include "Nick.h"
#include "Modules.h"
#include "Chan.h"

class CChanAttach : public CModule {
public:
	MODCONSTRUCTOR(CChanAttach) {}

	virtual ~CChanAttach() {
	}

	virtual bool OnLoad(const CString& sArgs) {
		unsigned int a = 0;
		CString sChan = sArgs.Token(a++);

		while (!sChan.empty()) {
			if (!Add(sChan)) {
				PutModule("Unable to add [" + sChan + "]");
			}

			sChan = sArgs.Token(a++);
		}

		return true;
	}

	void TryAttach(CChan& Channel) {
		const CString& sChan = Channel.GetName();

		if (Channel.IsDetached() && IsAutoAttach(sChan)) {
			Channel.JoinUser();
		}
	}

	virtual EModRet OnChanNotice(CNick& Nick, CChan& Channel, CString& sMessage) {
		TryAttach(Channel);
		return CONTINUE;
	}

	virtual EModRet OnChanMsg(CNick& Nick, CChan& Channel, CString& sMessage) {
		TryAttach(Channel);
		return CONTINUE;
	}

	virtual void OnModCommand(const CString& sLine) {
		CString sCommand = sLine.Token(0);

		if (sCommand.CaseCmp("ADD") == 0) {
			CString sChan = sLine.Token(1);

			if (Add(sChan)) {
				PutModule("Added " + sChan + " to list");
			} else {
				PutModule("Usage: Add [!]<#chan>");
			}
		} else if (sCommand.CaseCmp("LIST") == 0) {
			CTable Table;
			Table.AddColumn("Chan");

			for (unsigned int a = 0; a < m_vsChans.size(); a++) {
				Table.AddRow();
				Table.SetCell("Chan", m_vsChans[a]);
			}

			for (unsigned int b = 0; b < m_vsNegChans.size(); b++) {
				Table.AddRow();
				Table.SetCell("Chan", "!" + m_vsNegChans[b]);
			}

			if (Table.size()) {
				unsigned int uTableIdx = 0;
				CString sLine;

				while (Table.GetLine(uTableIdx++, sLine)) {
					PutModule(sLine);
				}
			} else {
				PutModule("You have no entries.");
			}
		} else if (sCommand.CaseCmp("HELP") == 0) {
			CTable Table;
			Table.AddColumn("Command");
			Table.AddColumn("Description");

			Table.AddRow();
			Table.SetCell("Command", "Add");
			Table.SetCell("Description", "Add an entry, use !#chan to negate and * for wildcards");

			Table.AddRow();
			Table.SetCell("Command", "List");
			Table.SetCell("Description", "List all entries");

			if (Table.size()) {
				unsigned int uTableIdx = 0;
				CString sLine;

				while (Table.GetLine(uTableIdx++, sLine)) {
					PutModule(sLine);
				}
			} else {
				PutModule("You have no entries.");
			}
		}
	}

	bool Add(const CString& sChan) {
		if (sChan.empty() || sChan == "!") {
			return false;
		}

		if (sChan.Left(1) == "!") {
			m_vsNegChans.push_back(sChan.substr(1));
		} else {
			m_vsChans.push_back(sChan);
		}

		return true;
	}

	bool IsAutoAttach(const CString& sChan) {
		for (unsigned int a = 0; a < m_vsNegChans.size(); a++) {
			if (sChan.WildCmp(m_vsNegChans[a])) {
				return false;
			}
		}

		for (unsigned int b = 0; b < m_vsChans.size(); b++) {
			if (sChan.WildCmp(m_vsChans[b])) {
				return true;
			}
		}

		return false;
	}
private:
	vector<CString>	m_vsChans;
	vector<CString>	m_vsNegChans;
};

MODULEDEFS(CChanAttach, "Reattaches you to channels on activity.")