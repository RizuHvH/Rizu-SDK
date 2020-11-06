struct animfix;
class animfix
{
public:
	void build_server_bones(IBasePlayer* player);
	void update_player(IBasePlayer* player, animation* record, animation* previous);
};
extern animfix* animfix;