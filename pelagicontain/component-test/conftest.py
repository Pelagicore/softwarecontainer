import pytest

def pytest_addoption(parser):
    parser.addoption("--pelagicontain_binary", action="store", default=None,
        help="Path to pelagicontain binary")
    parser.addoption("--container_path", action="store", default=None,
        help="Path to container")

@pytest.fixture(scope="module")
def pelagicontain_binary(request):
    return request.config.getoption("--pelagicontain_binary")

@pytest.fixture(scope="module")
def container_path(request):
    return request.config.getoption("--container_path")

@pytest.fixture
def teardown_fixture(request):
    """ Teardown run after each test run (both successful and failed).
        Pelagicontain will be shutdown if there is an assertion failure in the
        running test.

        Pass this fixture to any test function that should shutdown Pelagicontain
        upon assertion failure.
    """
    helper = getattr(request.module, "helper")
    def teardown():
        try:
            if request.node.rep_setup.failed:
                print "Setup failed, shutting down Pelagicontain"
                helper.teardown()
        except AttributeError as e:
            pass

        try:
            if request.node.rep_call.failed:
                print "A test failed, shutting down Pelagicontain"
                helper.teardown()
        except AttributeError as e:
            pass

    request.addfinalizer(teardown)

@pytest.mark.tryfirst
def pytest_runtest_makereport(item, call, __multicall__):
    """ This is a helper function that may be used from within other fixtures
        to determine whether setup, a test, or teardown has failed.

        The report attribute is accessed through the requesting context object
        in a fixture, e.g.:
        \code if request.node.rep_setup.failed: do_something()
        or
        \code if request.node.rep_call.failed: do_something()

        Example from: https://pytest.org/latest/example/simple.html
    """
    # Execute all other hooks to obtain the report object
    rep = __multicall__.execute()

    # Set a report attribute for each phase of a call, which can be "setup", "call", "teardown"
    setattr(item, "rep_" + rep.when, rep)
    return rep
