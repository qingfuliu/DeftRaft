//
// Created by lqf on 25-3-6.
//

#ifndef DEFTRAFT_TYPE_H
#define DEFTRAFT_TYPE_H

#include <cstdint>
#include "raft.pb.h"

namespace DeftRaft {
    using Entrys = std::vector<DeftRaft::Entry>;

    template<class T>
    struct MaybeValue {
        bool nonnull{false};
        T value{};
    };

    using MaybeUInt64t = MaybeValue<std::uint64_t>;

    struct EntryID {
        std::uint64_t m_index_;
        std::uint64_t m_term_;
    };

    EntryID EntryId(const DeftRaft::Entry &entry) {
        return EntryID{entry.index(), entry.term()};
    }

}

#endif //DEFTRAFT_TYPE_H
