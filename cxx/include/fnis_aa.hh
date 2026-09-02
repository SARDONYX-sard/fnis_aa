#pragma once

namespace FNIS_aa {
    bool Register(RE::BSScript::IVirtualMachine* vm);
}  // namespace FNIS_aa

namespace fnis_aa::menu {
    void UpdateSnapshot();
    void Register();
}
