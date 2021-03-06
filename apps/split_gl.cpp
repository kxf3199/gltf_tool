// split_gl.cpp: 定义控制台应用程序的入口点。
//

#include "beyon_3dtile.h"
using namespace std::literals;
using ygl::glTF;

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

	ygl::Beyon3dtile beyon3dtile(ygl::TileType::B3DM); 
	double lon = 116.39123, lat = 39.90691, height = 100;

 	beyon3dtile.generate_tileset(gltf_origin, output, ygl::degree2rad(lon),
             ygl::degree2rad(lat), height);
	//std::vector<glTF*> gltf_group = ygl::split_gltf(gltf_origin);

//     std::string int_path = ygl::path_dirname(filename);
//     ygl::save_gltf(int_path + "origin.gltf", gltf_origin, true);
    // 
//     ygl::glTF* gltf_origin = new ygl::glTF;
//     std::vector<glTF*> gltf_group = ygl::split_gltf(filename, gltf_origin);
//     std::string oupt_path = ygl::path_dirname(output);
//         for (auto gltf : gltf_group) {
//             std::string output_file = oupt_path + gltf->nodes[0]->name+".gltf";
//             ygl::save_gltf(output_file, gltf, true);
//         }
	
	
	
	

    return 0;
}

