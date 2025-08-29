#include <iostream>
#include <type_traits>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

class Variant {
public:
	enum Type {
		TYPE_NIL,
		TYPE_BOOL,
		TYPE_INT,
		TYPE_STRING
	};

private:
	Type type;
	union {
		bool _bool;
		int _int;
	} data;
	std::string _string;

public:
	Variant() : type(TYPE_NIL) {}
	Variant(bool b) : type(TYPE_BOOL) { data._bool = b; }
	Variant(int i) : type(TYPE_INT) { data._int = i; }
	Variant(const std::string& s) : type(TYPE_STRING), _string(s) {}
	Variant(const char* s) : type(TYPE_STRING), _string(s) {}

	Type get_type() const { return type; }

	bool as_bool() const { return type == TYPE_BOOL ? data._bool : false; }
	int as_int() const { return type == TYPE_INT ? data._int : 0; }
	std::string as_string() const { return type == TYPE_STRING ? _string : ""; }

	// Serialize to string
	std::string serialize() const {
		switch (type) {
			case TYPE_NIL: return "nil";
			case TYPE_BOOL: return std::string("bool:") + (data._bool ? "true" : "false");
			case TYPE_INT: return "int:" + std::to_string(data._int);
			case TYPE_STRING: return "string:" + _string;
			default: return "";
		}
	}

	// Deserialize from string
	static Variant deserialize(const std::string& str) {
		if (str == "nil") return Variant();
		if (str.rfind("bool:", 0) == 0) {
			return Variant(str.substr(5) == "true");
		}
		if (str.rfind("int:", 0) == 0) {
			return Variant(std::stoi(str.substr(4)));
		}
		if (str.rfind("string:", 0) == 0) {
			return Variant(str.substr(7));
		}
		// fallback to nil
		return Variant();
	}
};

class Object {
private:
    static int next_id;
    int id;

    // Signal system
    using SignalHandler = std::function<void(const Variant&)>;
    std::unordered_map<std::string, std::vector<SignalHandler>> signals;

public:
    Object() : id(next_id++) {}
    virtual ~Object() {}

    int get_id() const { return id; }

    // Connect a handler to a signal
    void connect(const std::string& signal_name, SignalHandler handler) {
        signals[signal_name].push_back(handler);
    }

    // Emit a signal
    void emit_signal(const std::string& signal_name, const Variant& value) {
        auto it = signals.find(signal_name);
        if (it != signals.end()) {
            for (auto& handler : it->second) {
                handler(value);
            }
        }
    }
};

int Object::next_id = 1;

int main() {
    Variant v1 = Variant("The number is: ");
    Variant v2 = Variant(42);

    std::cout << v1.serialize() << std::endl;

    // Example usage of Object
    Object obj1;
    Object obj2;
    std::cout << "Object 1 ID: " << obj1.get_id() << std::endl;
    std::cout << "Object 2 ID: " << obj2.get_id() << std::endl;

	// Signal example
    obj1.connect("print", [](const Variant& v) {
        std::cout << "Signal received: " << v.serialize() << std::endl;
    });
    obj1.emit_signal("print", Variant("Hello from signal!"));

    return 0;
}
