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

