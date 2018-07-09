#include "beyon_3dtile.h"

// -----------------------------------------------------------------------------
// IMPLEMENTATION FOR beyon_3dtile HIGH-LEVEL INTERFACE
// -----------------------------------------------------------------------------

namespace ygl {
B3dModel::B3dModel(const glTF* gl, FeatureTable& ft, BatchTable& bt)
    : gltf(gl),featureTable(ft),batchTable(bt) {
    std::string ft_str = ft.align();
    std::string bt_str = bt.align();
    header.featureJsonLen = ft_str.size();
    header.batchJsonLen = bt_str.size();


}
B3dModel::~B3dModel() { gltf = nullptr; };
	
Beyon3dtile::Beyon3dtile(TileType type, Tileset& ts)
    : m_tileType(type), m_tileSet(&ts) {
	
}
Beyon3dtile::~Beyon3dtile() {}

bool Beyon3dtile::save_tileset() {
	if (m_tileSet) { 
		json js=m_tileSet->convert_js();
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
    if (m_tileSet&&m_tileSet->m_root) m_tileSet->m_root->save_b3dm();
}
}





