#include <SequenceNode.h>


BT::SequenceNode::SequenceNode(std::string name) : ControlNode::ControlNode(name)
{
    // Thread start
    Thread = boost::thread(&SequenceNode::Exec, this);
}

BT::SequenceNode::~SequenceNode() {}

void BT::SequenceNode::Exec()
{
    unsigned int i;

    // Waiting for the first tick to come
    Semaphore.Wait();

    // Vector size initialization
    M = ChildNodes.size();

    // Simulating a tick for myself
    Semaphore.Signal();

    while(true)
    {
        // Waiting for a tick to come
        Semaphore.Wait();

        if(ReadState() == BT::Exit)
        {
            // The behavior tree is going to be destroied
            return;
        }

        // Checking if i was halted
        if (ReadState() != BT::Halted)
        {
            // If not, the children can be ticked
            std::cout << get_name() << " ticked, ticking children..." << std::endl;

            // For each child:
            for (i = 0; i<M; i++)
            {
                if (ChildNodes[i]->get_type() == BT::Action)
                {
                    // 1) if it's an action:
                    // 1.1) read its state;
                    NodeState ActionState = ChildNodes[i]->ReadState();

                    if (ActionState == BT::Idle)
                    {
                        // 1.2) if it's "Idle":
                        // 1.2.1) ticking it;
                        ChildNodes[i]->Semaphore.Signal();

                        // 1.2.2) retrive its state as soon as it is available;
                        ChildStates[i] = ChildNodes[i]->GetNodeState();
                    }
                    else if (ActionState == BT::Running)
                    {
                        // 1.3) if it's "Running":
                        // 1.3.1) saving "Running"
                        ChildStates[i] = BT::Running;
                    }
                    else
                    {
                        // 1.4) if it's "Success" of "Failure" (it can't be "Halted"!):
                        // 1.2.1) ticking it;
                        ChildNodes[i]->Semaphore.Signal();

                        // 1.2.2) saving the read state;
                        ChildStates[i] = ActionState;
                    }
                }
                else
                {
                    // 2) if it's not an action:
                    // 2.1) ticking it;
                    ChildNodes[i]->Semaphore.Signal();

                    // 2.2) retrive its state as soon as it is available;
                    ChildStates[i] = ChildNodes[i]->GetNodeState();
                }

                // 3) if the child state is not a success:
                if(ChildStates[i] != BT::Success)
                {
                    // 3.1) the node state is equal to it;
                    SetNodeState(ChildStates[i]);

                    // 3.2) state reset;
                    WriteState(BT::Idle);

                    // 3.3) all the next action or control child nodes must be halted:
                    HaltChildren(i+1);

                    std::cout << get_name() << " returning " << ChildStates[i] << "!" << std::endl;

                    // 3.4) the "for" loop must end here.
                    break;
                }
            }

            if (i == M)
            {
                // 4) if all of its children return "success":
                // 4.1) the node state must be "success";
                SetNodeState(BT::Success);

                // 4.2) resetting the state;
                WriteState(BT::Idle);

                std::cout << get_name() << " returning " << BT::Success << "!" << std::endl;
            }
        }
        else
        {
            // If it was halted, all the "busy" children must be halted too
            std::cout << get_name() << " halted! Halting all the children..." << std::endl;

            HaltChildren(0);
            // Resetting the node state
            WriteState(BT::Idle);
        }
    }
}

int BT::SequenceNode::GetType()
{
    // Lock acquistion

    return BT::SEQUENCE;
}

