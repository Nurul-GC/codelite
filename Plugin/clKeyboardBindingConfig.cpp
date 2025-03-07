#include "clKeyboardBindingConfig.h"

#include "JSON.h"
#include "cl_standard_paths.h"

#include <wx/filename.h>

clKeyboardBindingConfig::clKeyboardBindingConfig() {}

clKeyboardBindingConfig::~clKeyboardBindingConfig() {}

clKeyboardBindingConfig& clKeyboardBindingConfig::Load()
{
    wxFileName fn(clStandardPaths::Get().GetUserDataDir(), "keybindings.conf");
    fn.AppendDir("config");
    if(!fn.Exists()) {
        return *this;
    }

    m_bindings.clear();
    JSON root(fn);

    {
        JSONItem menus = root.toElement().namedObject("menus");
        int arrSize = menus.arraySize();
        for(int i = 0; i < arrSize; ++i) {
            JSONItem item = menus.arrayItem(i);
            MenuItemData binding;
            binding.resourceID = item.namedObject("resourceID").toString();
            binding.parentMenu = item.namedObject("parentMenu").toString();
            binding.action = item.namedObject("description").toString();
            binding.accel.FromString(item.namedObject("accelerator").toString());
            MigrateOldResourceID(binding.resourceID);
            m_bindings.insert(std::make_pair(binding.resourceID, binding));
        }
    }
    return *this;
}

clKeyboardBindingConfig& clKeyboardBindingConfig::Save()
{
    JSON root(cJSON_Object);
    JSONItem mainObj = root.toElement();
    JSONItem menuArr = JSONItem::createArray("menus");
    mainObj.append(menuArr);
    for(MenuItemDataMap_t::iterator iter = m_bindings.begin(); iter != m_bindings.end(); ++iter) {
        JSONItem binding = JSONItem::createObject();
        binding.addProperty("resourceID", iter->second.resourceID);
        binding.addProperty("parentMenu", iter->second.parentMenu);
        binding.addProperty("description", iter->second.action);
        binding.addProperty("accelerator", iter->second.accel.ToString());
        menuArr.arrayAppend(binding);
    }
    wxFileName fn(clStandardPaths::Get().GetUserDataDir(), "keybindings.conf");
    fn.AppendDir("config");
    root.save(fn);
    return *this;
}

void clKeyboardBindingConfig::MigrateOldResourceID(wxString& resourceID) const
{
    if(resourceID == "text_word_complete") {
        // This entry was moved from Word Completion plugin to CodeLite Edit menu entry
        resourceID = "simple_word_completion";
    } else if(resourceID == "word_complete") {
        resourceID = "complete_word";
    }
}
