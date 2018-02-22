#include <testeth/TestHelper.h>
#include <testeth/TestOutputHelper.h>
#include <testeth/Options.h>

using namespace std;
namespace fs = boost::filesystem;

namespace  test {

vector<fs::path> getFiles(
	fs::path const& _dirPath, set<string> const _extentionMask, string const& _particularFile)
{
	vector<fs::path> files;
	for (auto const& ext : _extentionMask)
	{
		if (!_particularFile.empty())
		{
			fs::path file = _dirPath / (_particularFile + ext);
			if (fs::exists(file))
				files.push_back(file);
		}
		else
		{
			using fsIterator = fs::directory_iterator;
			for (fsIterator it(_dirPath); it != fsIterator(); ++it)
			{
				if (fs::is_regular_file(it->path()) && it->path().extension() == ext)
					files.push_back(it->path());
			}
		}
	}
	return files;
}

boost::filesystem::path getTestPath()
{
	if (!Options::get().testpath.empty())
		return Options::get().testpath;

	string testPath;
	const char* ptestPath = getenv("ETHEREUM_TEST_PATH");

	if (ptestPath == nullptr)
	{
//		ctest << " could not find environment variable ETHEREUM_TEST_PATH \n";
		testPath = "../../test/jsontests";
	}
	else
		testPath = ptestPath;

	return boost::filesystem::path(testPath);
}

void copyFile(fs::path const& _source, fs::path const& _destination)
{
	fs::ifstream src(_source, ios::binary);
	fs::ofstream dst(_destination, ios::binary);
	dst << src.rdbuf();
}

void requireJsonFields(DataObject const& _o, string const& _section,
	map<string, DataType> const& _validationMap)
{
	// check for unexpected fiedls
	for (auto const field : _o.getSubObjects())
		BOOST_CHECK_MESSAGE(_validationMap.count(field.getKey()),
			field.getKey() + " should not be declared in " + _section + " section!");

	// check field types with validation map
	for (auto const vmap : _validationMap)
	{
		BOOST_REQUIRE_MESSAGE(_o.count(vmap.first) > 0, vmap.first + " not found in " + _section +
															" section! " +
															TestOutputHelper::get().testName());
		BOOST_REQUIRE_MESSAGE(_o.at(vmap.first).type() == vmap.second,
			_section + " " + vmap.first + " expected to be " + DataObject::dataTypeAsString(vmap.second) +
				", but set to " + DataObject::dataTypeAsString(_o.at(vmap.first).type()) + " in " +
				TestOutputHelper::get().testName());
	}
}


string jsonTypeAsString(Json::ValueType _type)
{
	switch (_type) {
	case Json::ValueType::nullValue: return "null";
	case Json::ValueType::intValue: return "int";
	case Json::ValueType::uintValue: return "uint";
	case Json::ValueType::realValue: return "double";
	case Json::ValueType::stringValue: return "string";
	case Json::ValueType::booleanValue: return "bool";
	case Json::ValueType::arrayValue: return "array";
	case Json::ValueType::objectValue: return "object";
	default:
		break;
	}
	return "";
}

DataObject convertJsonCPPtoData(Json::Value const& _input)
{
	if (_input.isNull())
		return DataObject(DataType::Null);

	if (_input.isString())
		return DataObject(_input.asString());

	if (_input.isInt())
		return DataObject(_input.asInt());

	if (_input.isArray())
	{
		DataObject root(DataType::Array);
		for (Json::ArrayIndex i = 0; i < _input.size(); i++)
			root.addArrayObject(convertJsonCPPtoData(_input.get(i, Json::Value())));
		return root;
	}

	if (_input.isObject())
	{
		DataObject root(DataType::Object);
		for (auto const& i: _input)
			root.addSubObject(convertJsonCPPtoData(i));

		size_t index = 0;
		for (auto const& i: _input.getMemberNames())
			root.setSubObjectKey(index++, i);
		return root;
	}

	BOOST_ERROR("Error parsing JSON node. Element type not defined! " + jsonTypeAsString(_input.type()));
	return DataObject(DataType::Null);
}


}
