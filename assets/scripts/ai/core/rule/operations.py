from typing import List

from ai.core.rule.memory import *

def handle_any_bool_operators(*operations) -> List[OperationBase]:
    ops_to_return: List[OperationBase] = []

    for operation in operations:
        if isinstance(operation, Fact):
            operation = operation.as_bool()

        if isinstance(operation, Memory):
            operation = operation.as_bool()

        ops_to_return.append(operation)
    return ops_to_return


class AndOperation(OperationBase):
    def __init__(self, *operations):
        self.operations : List[OperationBase] = handle_any_bool_operators(*operations)

    @override
    def is_true(self, context: Context) -> bool:
        for operation in self.operations:
            if not operation.is_true(context):
                return False
        return True


class OrOperation(OperationBase):
    def __init__(self, *operations):
        self.operations : List[OperationBase] = handle_any_bool_operators(*operations)

    @override
    def is_true(self, context: Context) -> bool:
        for operation in self.operations:
            if operation.is_true(context):
                return True
        return False


class NotOperation(OperationBase):
    def __init__(self, operation):
        self.operation : OperationBase = handle_any_bool_operators(operation)[0]

    @override
    def is_true(self, context: Context) -> bool:
        return not self.operation.is_true(context)


class AlwaysFalseOperation(OperationBase):
    @override
    def is_true(self, context: Context) -> bool:
        context.errors.append(
            Error("always_false", None, None, None, None))
        return False


class AlwaysTrueOperation(OperationBase):
    @override
    def is_true(self, context: Context) -> bool:
        return True


Not = NotOperation
Or = OrOperation
And = AndOperation
When = AndOperation
always_false = AlwaysFalseOperation()
always_true = AlwaysTrueOperation()