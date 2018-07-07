// split_gl.cpp: 定义控制台应用程序的入口点。
//

#include "../yocto/yocto_gltf.h"
using namespace std::literals;
using ygl::glTF;
using ygl::mat4f;
enum BoundingStyle
{
	region,
	box,
	sphere
};
enum RefineStyle
{
	ADD,
	REPLACE
};
struct STR_BoundingVolume {
	BoundingStyle m_style = box;
	double* m_dArray = NULL;
	size_t m_len = 0;
};
struct STR_TileContent
{
	std::string m_tileUrl;
};
struct STR_TileNode
{
	STR_BoundingVolume m_boundingVol;
	mat4f m_transform;
	float m_geometricError;
	RefineStyle m_refineStyle;
	STR_TileContent m_content;
};
struct STR_Asset {
	std::string m_version = "0.0";
	std::string m_tilesetVersion;
	std::string m_glftUpAxis = "Y";
};
struct STR_Tileset
{
	STR_Asset m_asset;
	float m_geometricError = 200;
	STR_TileNode m_root;
};
int main(int argc, char** argv) {
    auto parser =
        ygl::make_parser(argc, argv, "split_gl", "split a gltf");
    auto filename = ygl::parse_opt(
        parser, "--input", "-i", "output scene filename", "input.gltf"s);
    auto output = ygl::parse_opt(
        parser, "--output", "-o", "output scene filename", "out.obj"s);


    if (ygl::should_exit(parser)) {
        printf("%s\n", get_usage(parser).c_str());
        exit(1);
    }

	//std::vector<glTF*> gltf_group;
    auto ext = ygl::path_extension(filename);
    auto gltf_ptr = std::unique_ptr<glTF>();
    if (ext != ".glb") {
        gltf_ptr = std::unique_ptr<glTF>(ygl::load_gltf(filename, true, true, true));
    } else {
        gltf_ptr = std::unique_ptr<glTF>(
            ygl::load_binary_gltf(filename, true, true, true));
    }
    glTF* gltf_origin = gltf_ptr.get();

	std::vector<glTF*> gltf_group = ygl::split_gltf(gltf_origin);

//     std::string int_path = ygl::path_dirname(filename);
//     ygl::save_gltf(int_path + "origin.gltf", gltf_origin, true);
    // 
//     ygl::glTF* gltf_origin = new ygl::glTF;
//     std::vector<glTF*> gltf_group = ygl::split_gltf(filename, gltf_origin);
    std::string oupt_path = ygl::path_dirname(output);
        for (auto gltf : gltf_group) {
            std::string output_file = oupt_path + gltf->nodes[0]->name+".gltf";
            ygl::save_gltf(output_file, gltf, true);
        }
	
	
	
	

    return 0;
}

