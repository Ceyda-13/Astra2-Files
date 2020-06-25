#pragma once

class PlayerHB : public singleton<PlayerHB>
{
    public:
	PlayerHB();
	virtual ~PlayerHB();

	bool	Initialize();

	void	Put(uint32_t id);

    private:
	bool	Query(uint32_t id);

	std::map<uint32_t, time_t> m_map_data;
	std::string		m_stCreateTableQuery;
	std::string		m_stTableName;
	int32_t			m_iExpireTime;
};


