#include <Glacier/Fysix/Fysix.h>


namespace Glacier::Fysix
{
    ZConstraintGroup::~ZConstraintGroup()
    {
        // TODO: Finish me
    }

    void ZConstraintGroup::NormalSolver(const uint32_t lCount, const ZConstraintBody& body, const SConstraintProps& prop, const SConstraintIndex& idx)
    {
        // TODO: Finish me
    }

    void ZConstraintGroup::QuickSolver(uint32_t lCount, const ZConstraintBody& body, const SConstraintProps& prop, const SConstraintIndex& idx)
    {
        // TODO: Finish me
    }

    void ZConstraintGroup::LinkSolver(const SLinkage& con)
    {
        // TODO: Finish me
    }

    void ZConstraintGroup::HandleFracture(const ZConstraintBody& body, uint16_t& id, float(&velocity)[3])
    {
        // TODO: Finish me
    }

    void ZConstraintGroup::Strain(const ZConstraintBody& body, uint16_t& amount, uint16_t& strainPar)
    {
        // TODO: Finish me
    }
}