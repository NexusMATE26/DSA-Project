#pragma once

#include <cstdint>
#include "internpath/ds/Stack.h"
#include "internpath/domain/Types.h"

namespace internpath
{
    struct ProfileEdit
    {
        StudentId studentId = 0;
        SkillId skillId = 0;
        std::uint8_t oldLevel = 0;
        std::uint8_t newLevel = 0;
    };

    class UndoHistory
    {
    private:
        Stack<ProfileEdit> changes_;

    public:
        // TODO A25: record edit only after success; pop on undo.
        UndoHistory() = default;

        bool empty() const
        {
            return changes_.empty();
        }

        std::size_t size() const
        {
            return changes_.size();
        }

        void recordEdit(const ProfileEdit& edit)
        {
            changes_.push(edit);
        }

        bool popUndo(ProfileEdit& outEdit)
        {
            return changes_.pop(outEdit);
        }

        void clear()
        {
            changes_.clear();
        }
    };
}
