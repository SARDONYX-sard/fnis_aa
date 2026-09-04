#pragma once

namespace fnis_aa::FNIS_aa2 {
    bool Register(RE::BSScript::IVirtualMachine* vm);
}

namespace fnis_aa::FNIS_aa {
    bool Register(RE::BSScript::IVirtualMachine* vm);
}

namespace fnis_aa::FNIS {
    bool Register(RE::BSScript::IVirtualMachine* vm);
}

namespace fnis_aa::menu {
    void UpdateSnapshot();
    void Register();
}
