#include "beyon_3dtile.h"
#include "../yocto/yocto_gltf.h"
// -----------------------------------------------------------------------------
// IMPLEMENTATION FOR beyon_3dtile HIGH-LEVEL INTERFACE
// -----------------------------------------------------------------------------

namespace ygl {



B3dModel::B3dModel(const glTF* gl, FeatureTable& ft, BatchTable& bt)
    : gltf(gl),featureTable(ft),batchTable(bt) {
    std::string ft_str = "", bt_str = "";
    header.featureJsonLen = ft.align(ft_str);
    header.batchJsonLen = bt.align(bt_str);
    ygl::gltf_to_glb(gl, glb);
	int glbByteLen = glb.size();
    header.byteLen = 28 + header.featureJsonLen + header.featureBinLen +
                          header.batchJsonLen + header.batchBinLen + glbByteLen;
}
B3dModel::~B3dModel() { gltf = nullptr; };
	
Beyon3dtile::Beyon3dtile(TileType type) : m_tileType(type) {
    m_tileSet = new Tileset;
}
Beyon3dtile::~Beyon3dtile() {
    delete m_tileSet;
}

bool Beyon3dtile::save_tileset() {
	if (m_tileSet) { 

		json js;
            m_tileSet->convert_js(js);
            // fix string
        auto js_str = js.dump(2);	
		std::string filename = m_tileSet->m_basePath + "tileset.json";
        auto f = fopen(filename.c_str(), "wb");
        if (!f) throw std::runtime_error("cannot write file " + filename);
        fwrite(js_str.data(), 1, js_str.size(), f);
        fclose(f);
        
	}

	return true;

}
void Beyon3dtile::save_model() {
    if (m_tileSet && m_tileSet->m_root) m_tileSet->m_root->save_b3dm(m_tileSet->m_basePath);
}

bool Beyon3dtile::generate_tileset(glTF* gl, std::string outpath,
    double radian_x, double radian_y, double height, double tile_w,
    double tile_h,double height_min, double height_max, double geometricError) {
    std::vector<glTF*> group_gltf = split_gltf(gl);
    m_tileSet->m_basePath = outpath;
    m_tileSet->m_refineStyle = REPLACE;
    m_tileSet->m_root = new TileNode;
    auto get_bound = [](std::vector<float>& max, std::vector<float>& min,
                         const std::vector<float>& pos) {
        if (max[0] < pos[0]) max[0] = pos[0];
        if (max[1] < pos[1]) max[1] = pos[1];
        if (max[2] < pos[2]) max[2] = pos[2];
        if (min[0] > pos[0]) min[0] = pos[0];
        if (min[1] > pos[1]) min[1] = pos[1];
        if (min[2] > pos[2]) min[2] = pos[2];
    };
    m_tileSet->m_root->m_boundingVol.style = SPHERE;
    std::vector<float> root_max = {FLT_MIN, FLT_MIN, FLT_MIN},
                       root_min = {FLT_MAX, FLT_MAX, FLT_MAX};
    vec3f maxf, minf;
    for (auto gl : group_gltf) {
        merge_buffer(*gl);
        BatchTable batTable;
        std::vector<float> max = {FLT_MIN, FLT_MIN, FLT_MIN},
                           min = {FLT_MAX, FLT_MAX, FLT_MAX};
        for (auto gBatch : gl->batches) {
            batTable.js["batchId"].push_back(gBatch->id);
            batTable.js["name"].push_back(gBatch->name);
            batTable.js["maxPoint"].push_back(gBatch->maxPoint);
            get_bound(max, min, gBatch->maxPoint);
            batTable.js["minPoint"].push_back(gBatch->minPoint);
            get_bound(max, min, gBatch->minPoint);
        }

        TileNode* model_node = new TileNode;
        model_node->m_boundingVol.style = SPHERE;
        maxf = vec3f(max[0], max[1], max[2]);
        minf = vec3f(min[0], min[1], min[2]);
        vec3f center = (maxf + minf) / 2;
        model_node->m_boundingVol.boundArray.resize(4);
        memcpy(model_node->m_boundingVol.boundArray.data(), data(center),
            sizeof(float) * size(center));
        float radius = sqrtf((maxf.x - minf.x) * (maxf.x - minf.x) / 4 +
                             (maxf.y - minf.y) * (maxf.y - minf.y) / 4 +
                             (maxf.z - minf.z) * (maxf.z - minf.z) / 4);
        model_node->m_boundingVol.boundArray[3] = radius;
        model_node->m_content.uri = "./" + gl->nodes[0]->name + ".b3dm";
        model_node->m_refineStyle = REPLACE;

        FeatureTable feaTable;
        feaTable.js["BATCH_LENGTH"] = batTable.js["batchId"].size();

        B3dModel* b3dModel = new B3dModel(gl, feaTable, batTable);
        model_node->m_b3dm = b3dModel;

        m_tileSet->m_root->m_childNode.push_back(model_node);
        get_bound(root_max, root_min, max);
        get_bound(root_max, root_min, min);
    }
    maxf = vec3f(root_max[0], root_max[1], root_max[2]);
    minf = vec3f(root_min[0], root_min[1], root_min[2]);
    vec3f center = (maxf + minf) / 2;
    m_tileSet->m_root->m_boundingVol.boundArray.resize(4);
    memcpy(m_tileSet->m_root->m_boundingVol.boundArray.data(), data(center),
        sizeof(float) * size(center));
    float radius = sqrtf((maxf.x - minf.x) * (maxf.x - minf.x) / 4 +
                         (maxf.y - minf.y) * (maxf.y - minf.y) / 4 +
                         (maxf.z - minf.z) * (maxf.z - minf.z) / 4);
    m_tileSet->m_root->m_boundingVol.boundArray[3] = radius;
    m_tileSet->m_root->m_transform =
        ygl::transfrom_xyz(radian_x, radian_y, height);
    save();
    return true;
}
void Beyon3dtile::set_pos(double lon, double lat, double height) {

}

}//namespace ylg







