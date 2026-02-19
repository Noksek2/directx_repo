#include "D3D.h"
class GameObject;
class IComponent {
public:
	GameObject* owner = nullptr;
};
class GameObject {
private:
protected:
	std::vector<std::unique_ptr<GameObject>> child;
	GameObject* parent = nullptr;
public:
};