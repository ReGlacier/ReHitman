#include <Glacier/Component/ZComponentProducer.h>
#include <Glacier/Component/ZComponentManagerProducer.h>
#include <Glacier/Component/ZComponentSingleton.h>
#include <Glacier/Component/ZComponentDefaultAllocator.h>

namespace Glacier
{
    struct TemplateCompileComponent : ZComponent<TemplateCompileComponent, ZComponentDescription<0, ZComponentDefaultAllocator>>
    {
        static ZComponentId ComponentId() { return "TemplateCompileComponent"; }
    };

    struct TemplateCompileGlobal : ZComponentSingleton<TemplateCompileGlobal, ZGlobalComponentBase>
    {
        static ZComponentId ComponentId() { return "TemplateCompileGlobal"; }
    };

    static_assert(ZComponentProducer<TemplateCompileComponent>::GROUP_ID == 0);
    static_assert(ZComponentManagerProducer<TemplateCompileGlobal>::GROUP_ID == 1);
    using TemplateCompileProducer = ZComponentProducer<TemplateCompileComponent>;
    using TemplateCompileGlobalProducer = ZComponentProducer<TemplateCompileGlobal>;
    using TemplateCompileManagerProducer = ZComponentManagerProducer<TemplateCompileGlobal>;

    void InstantiateComponentTemplates()
    {
        char creationParameters[] = "";
        TemplateCompileProducer producer("TemplateCompileComponent", creationParameters);
        TemplateCompileGlobalProducer globalProducer("TemplateCompileGlobal", creationParameters);
        TemplateCompileManagerProducer managerProducer(nullptr);
        (void)producer;
        (void)globalProducer;
        (void)managerProducer;
    }
}
